#include "arkwriter.h"

#include <algorithm>
#include <cstdint>
#include <format>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <utility>

#include "arkreader.h"
#include "byteorder.h"
#include "imageerror.h"
#include "math/hash.h"

namespace Tools::Ark {

using Common::Error;
using Common::invalidInput;
using Common::ioError;
using Common::readLittle32;
using Common::writeLittle;

namespace {

// The directory slot of a file at the root.
constexpr std::uint32_t kRootSlot = 0xffffffff;
// The retail archives have about two and a half slots per name.
constexpr std::size_t kSlotsPerNameNumerator = 5;
constexpr std::size_t kSlotsPerNameDenominator = 2;
constexpr std::size_t kRecordWords = 5;
constexpr std::size_t kCopyChunk = 1 << 20;
constexpr std::string_view kGzipSuffix = ".gz";
// A gzip file ends with the size of its inflated data.
constexpr std::size_t kGzipSizeField = 4;
// The record offsets and sizes are signed 32-bit values in the game.
constexpr std::uint64_t kArchiveLimit = std::numeric_limits<std::int32_t>::max();

struct Source {
    std::string path;
    std::filesystem::path file;
    std::uint32_t size = 0;
    std::uint32_t uncompressedSize = 0;
    std::uint32_t nameSlot = 0;
    std::uint32_t directorySlot = kRootSlot;
};

class NameTable {
public:
    explicit NameTable(std::size_t names)
        : slots_(
              std::max<std::size_t>(1, names * kSlotsPerNameNumerator / kSlotsPerNameDenominator)) {
        // Offset zero marks an empty slot, so the block opens with an empty name.
        strings_.push_back('\0');
    }

    std::uint32_t slotOf(const std::string &name) {
        if (const auto found = interned_.find(name); found != interned_.end()) {
            return found->second;
        }
        const auto count = static_cast<std::uint32_t>(slots_.size());
        auto slot = static_cast<std::uint32_t>(HashString(name.c_str(), static_cast<int>(count)));
        while (slots_[slot] != 0) {
            slot = (slot + 1) % count;
        }
        slots_[slot] = static_cast<std::uint32_t>(strings_.size());
        strings_.insert(strings_.end(), name.begin(), name.end());
        strings_.push_back('\0');
        interned_.emplace(name, slot);
        return slot;
    }

    [[nodiscard]] const std::vector<char> &strings() const {
        return strings_;
    }

    [[nodiscard]] const std::vector<std::uint32_t> &slots() const {
        return slots_;
    }

private:
    std::vector<char> strings_;
    std::vector<std::uint32_t> slots_;
    std::map<std::string, std::uint32_t> interned_;
};

std::expected<std::uint32_t, Error> gzipSize(const std::filesystem::path &file,
                                             std::uint32_t size) {
    if (size < kGzipSizeField) {
        return invalidInput(std::format("{} is too short for a gzip file.", file.string()));
    }
    std::ifstream in(file, std::ios::binary);
    in.seekg(size - kGzipSizeField);
    std::uint8_t field[kGzipSizeField] = {};
    // The streams read char.
    in.read(reinterpret_cast<char *>(field), sizeof(field));
    if (!in) {
        return ioError(std::format("Cannot read {}.", file.string()));
    }
    return readLittle32(field, 0);
}

std::expected<std::vector<Source>, Error> collect(const std::filesystem::path &root,
                                                  const std::vector<std::string> &order) {
    std::map<std::string, Source> found;
    std::error_code error;
    for (std::filesystem::recursive_directory_iterator it(root, error), end; !error && it != end;
         it.increment(error)) {
        if (!it->is_regular_file(error)) {
            continue;
        }
        const auto path = it->path().lexically_relative(root).generic_string();
        const auto size = it->file_size(error);
        if (error || size > kArchiveLimit) {
            return invalidInput(std::format("{} cannot be stored.", it->path().string()));
        }
        Source source{path, it->path(), static_cast<std::uint32_t>(size)};
        if (path.ends_with(kGzipSuffix)) {
            auto inflated = gzipSize(it->path(), source.size);
            if (!inflated) {
                return std::unexpected(inflated.error());
            }
            source.uncompressedSize = *inflated;
        }
        found.emplace(path, std::move(source));
    }
    if (error) {
        return ioError(std::format("Cannot list {}.", root.string()));
    }
    std::vector<Source> sources;
    std::set<std::string> placed;
    for (const auto &path : order) {
        if (const auto source = found.find(path);
            source != found.end() && placed.insert(path).second) {
            sources.push_back(source->second);
        }
    }
    for (const auto &[path, source] : found) {
        if (!placed.contains(path)) {
            sources.push_back(source);
        }
    }
    return sources;
}

void putWords(std::ofstream &out, std::initializer_list<std::uint32_t> words) {
    for (const auto word : words) {
        std::uint8_t bytes[sizeof(std::uint32_t)] = {};
        writeLittle(bytes, 0, word);
        // The streams write char.
        out.write(reinterpret_cast<const char *>(bytes), sizeof(bytes));
    }
}

} // namespace

std::expected<std::size_t, Error> ArkWriter::write(const std::filesystem::path &root,
                                                   const std::vector<std::string> &order,
                                                   const std::filesystem::path &output) {
    auto sources = collect(root, order);
    if (!sources) {
        return std::unexpected(sources.error());
    }
    std::set<std::string> names;
    for (const auto &source : *sources) {
        const auto slash = source.path.rfind('/');
        names.insert(source.path.substr(slash + 1));
        if (slash != std::string::npos) {
            names.insert(source.path.substr(0, slash));
        }
    }
    NameTable table(names.size());
    for (auto &source : *sources) {
        const auto slash = source.path.rfind('/');
        source.nameSlot = table.slotOf(source.path.substr(slash + 1));
        if (slash != std::string::npos) {
            source.directorySlot = table.slotOf(source.path.substr(0, slash));
        }
    }
    // The version, the record count, the records, the string block with its size, and the hash
    // table with its size.
    std::uint64_t offset = (2 + (sources->size() * kRecordWords) + 1 + 1 + table.slots().size()) *
                               sizeof(std::uint32_t) +
                           table.strings().size();
    std::ofstream out(output, std::ios::binary | std::ios::trunc);
    if (!out) {
        return ioError(std::format("Cannot write {}.", output.string()));
    }
    putWords(out, {ArkReader::kVersion, static_cast<std::uint32_t>(sources->size())});
    for (const auto &source : *sources) {
        if (offset + source.size > kArchiveLimit) {
            return invalidInput(
                std::format("The archive passes {} bytes at {}.", kArchiveLimit, source.path));
        }
        putWords(out,
                 {static_cast<std::uint32_t>(offset),
                  source.nameSlot,
                  source.directorySlot,
                  source.size,
                  source.uncompressedSize});
        offset += source.size;
    }
    putWords(out, {static_cast<std::uint32_t>(table.strings().size())});
    out.write(table.strings().data(), static_cast<std::streamsize>(table.strings().size()));
    putWords(out, {static_cast<std::uint32_t>(table.slots().size())});
    for (const auto slot : table.slots()) {
        putWords(out, {slot});
    }
    std::vector<char> buffer(kCopyChunk);
    for (const auto &source : *sources) {
        std::ifstream in(source.file, std::ios::binary);
        for (std::uint32_t done = 0; done < source.size;) {
            const auto chunk = std::min<std::size_t>(buffer.size(), source.size - done);
            in.read(buffer.data(), static_cast<std::streamsize>(chunk));
            if (static_cast<std::size_t>(in.gcount()) != chunk) {
                return ioError(std::format("Cannot read {}.", source.file.string()));
            }
            out.write(buffer.data(), static_cast<std::streamsize>(chunk));
            done += static_cast<std::uint32_t>(chunk);
        }
    }
    out.close();
    if (!out) {
        return ioError(std::format("Cannot write {}.", output.string()));
    }
    return sources->size();
}

} // namespace Tools::Ark

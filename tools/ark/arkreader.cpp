#include "arkreader.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <utility>

#include "byteorder.h"
#include "imageerror.h"
#include "math/hash.h"

namespace Tools::Ark {

using Common::Error;
using Common::invalidInput;
using Common::ioError;
using Common::readLittle32;

namespace {

// The directory slot of a file at the root.
constexpr std::uint32_t kRootSlot = 0xffffffff;
// Fields of a record, in file order.
enum RecordField : std::size_t {
    kRecordOffset = 0,
    kRecordName = 1,
    kRecordDirectory = 2,
    kRecordSize = 3,
    kRecordUncompressedSize = 4,
    kRecordFields = 5,
};
constexpr std::size_t kCopyChunk = 1 << 20;

} // namespace

std::expected<ArkReader, Error> ArkReader::open(const std::filesystem::path &path,
                                                std::uint64_t base) {
    ArkReader reader;
    reader.path_ = path;
    reader.base_ = base;
    reader.file_.open(path, std::ios::binary);
    reader.file_.seekg(static_cast<std::streamoff>(base));
    if (!reader.file_) {
        return ioError(std::format("Cannot read {}.", path.string()));
    }
    auto &file = reader.file_;
    const auto words = [&file](std::size_t count) {
        std::vector<std::uint32_t> values(count);
        for (auto &value : values) {
            std::uint8_t bytes[sizeof(std::uint32_t)] = {};
            // The streams read char.
            file.read(reinterpret_cast<char *>(bytes), sizeof(bytes));
            value = readLittle32(bytes, 0);
        }
        return values;
    };
    const auto header = words(2);
    if (!file || header[0] != kVersion) {
        return invalidInput(
            std::format("{} is not a version {} archive.", path.string(), kVersion));
    }
    const auto records = words(std::size_t{header[1]} * kRecordFields);
    const auto stringsSize = words(1).front();
    reader.strings_.resize(stringsSize);
    file.read(reader.strings_.data(), static_cast<std::streamsize>(stringsSize));
    const auto slotCount = words(1).front();
    if (!file || slotCount == 0) {
        return invalidInput(std::format("The directory of {} is cut short.", path.string()));
    }
    reader.table_ = words(slotCount);
    if (!file) {
        return invalidInput(std::format("The hash table of {} is cut short.", path.string()));
    }
    const auto nameAt = [&reader, slotCount](std::uint32_t slot) -> std::optional<std::string> {
        if (slot == kRootSlot) {
            return std::string();
        }
        if (slot >= slotCount || reader.table_[slot] == 0 ||
            reader.table_[slot] >= reader.strings_.size()) {
            return std::nullopt;
        }
        const auto *start = reader.strings_.data() + reader.table_[slot];
        return std::string(start, strnlen(start, reader.strings_.size() - reader.table_[slot]));
    };
    for (std::size_t i = 0; i < header[1]; ++i) {
        const auto *record = records.data() + (i * kRecordFields);
        const auto name = nameAt(record[kRecordName]);
        const auto directory = nameAt(record[kRecordDirectory]);
        if (!name || !directory || name->empty()) {
            return invalidInput(std::format("Record {} of {} has no name.", i, path.string()));
        }
        reader.entries_.push_back({directory->empty() ? *name : *directory + '/' + *name,
                                   record[kRecordOffset],
                                   record[kRecordSize],
                                   record[kRecordUncompressedSize]});
        reader.slots_.push_back(record[kRecordName]);
        reader.slots_.push_back(record[kRecordDirectory]);
    }
    return reader;
}

const std::vector<ArkEntry> &ArkReader::entries() const {
    return entries_;
}

int ArkReader::slotOf(std::string_view name) const {
    const auto count = static_cast<std::uint32_t>(table_.size());
    const std::string key(name);
    for (auto slot = static_cast<std::uint32_t>(HashString(key.c_str(), static_cast<int>(count)));
         table_[slot] != 0;
         slot = (slot + 1) % count) {
        if (table_[slot] < strings_.size() && strings_.data() + table_[slot] == name) {
            return static_cast<int>(slot);
        }
    }
    return -1;
}

std::optional<ArkEntry> ArkReader::locate(std::string_view path) const {
    const auto slash = path.rfind('/');
    const auto name = slotOf(slash == std::string_view::npos ? path : path.substr(slash + 1));
    auto directory = static_cast<int>(kRootSlot);
    if (slash != std::string_view::npos) {
        directory = slotOf(path.substr(0, slash));
        if (directory < 0) {
            return std::nullopt;
        }
    }
    if (name < 0) {
        return std::nullopt;
    }
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        if (slots_[2 * i] == static_cast<std::uint32_t>(name) &&
            slots_[(2 * i) + 1] == static_cast<std::uint32_t>(directory)) {
            return entries_[i];
        }
    }
    return std::nullopt;
}

std::expected<std::vector<std::uint8_t>, Error> ArkReader::read(const ArkEntry &entry) {
    std::vector<std::uint8_t> data(entry.size);
    file_.clear();
    file_.seekg(static_cast<std::streamoff>(base_ + entry.offset));
    // The streams read char.
    file_.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    if (static_cast<std::size_t>(file_.gcount()) != data.size()) {
        return invalidInput(std::format("{} ends inside {}.", path_.string(), entry.path));
    }
    return data;
}

std::expected<void, Error> ArkReader::extract(const std::filesystem::path &root) {
    std::vector<char> buffer(kCopyChunk);
    for (const auto &entry : entries_) {
        const auto target = root / entry.path;
        std::error_code error;
        std::filesystem::create_directories(target.parent_path(), error);
        if (error) {
            return ioError(std::format("Cannot create {}.", target.parent_path().string()));
        }
        std::ofstream out(target, std::ios::binary | std::ios::trunc);
        file_.clear();
        file_.seekg(static_cast<std::streamoff>(base_ + entry.offset));
        for (std::uint32_t done = 0; done < entry.size;) {
            const auto chunk = std::min<std::size_t>(buffer.size(), entry.size - done);
            file_.read(buffer.data(), static_cast<std::streamsize>(chunk));
            if (static_cast<std::size_t>(file_.gcount()) != chunk) {
                return invalidInput(std::format("{} ends inside {}.", path_.string(), entry.path));
            }
            out.write(buffer.data(), static_cast<std::streamsize>(chunk));
            done += static_cast<std::uint32_t>(chunk);
        }
        out.close();
        if (!out) {
            return ioError(std::format("Cannot write {}.", target.string()));
        }
    }
    return {};
}

} // namespace Tools::Ark

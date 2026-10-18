#include "datafile.h"

#include <algorithm>
#include <format>
#include <iterator>
#include <utility>

#include "byteorder.h"
#include "imageerror.h"
#include "math/rand.h"

namespace Tools::Ark {

using Common::Error;
using Common::invalidInput;
using Common::readLittle16;
using Common::readLittle32;
using Common::writeLittle;

namespace {

// BinStream encrypts each byte with the low byte of the next draw of a Rand seeded by the file.
void applyCipher(std::uint32_t seed, std::span<std::uint8_t> bytes) {
    Rand cipher(static_cast<int>(seed));
    for (auto &byte : bytes) {
        byte ^= static_cast<std::uint8_t>(cipher.Int());
    }
}

constexpr std::size_t kSeedSize = 4;
// An array stores sixteen two-bit element tags in each 32-bit word.
constexpr std::size_t kTagsPerWord = 16;
constexpr std::uint32_t kTagMask = 3;
constexpr int kTagBits = 2;
// The deepest nesting parse() accepts, far beyond any file of the game.
constexpr int kMaxDepth = 256;

class Reader {
public:
    explicit Reader(std::span<const std::uint8_t> data) : data_(data) {
    }

    // The width is sizeof(std::uint16_t) or sizeof(std::uint32_t).
    std::expected<std::uint32_t, Error> word(std::size_t width = sizeof(std::uint32_t)) {
        if (width > data_.size() - offset_) {
            return invalidInput(std::format("The file ends at offset {:#x}.", offset_));
        }
        const auto value = width == sizeof(std::uint16_t) ? readLittle16(data_, offset_) :
                                                            readLittle32(data_, offset_);
        offset_ += width;
        return value;
    }

    std::expected<std::string, Error> string() {
        const auto length = word();
        if (!length) {
            return std::unexpected(length.error());
        }
        if (*length > data_.size() - offset_) {
            return invalidInput(std::format("A string at offset {:#x} passes the end.", offset_));
        }
        std::string text(data_.begin() + static_cast<std::ptrdiff_t>(offset_),
                         data_.begin() + static_cast<std::ptrdiff_t>(offset_ + *length));
        offset_ += *length;
        return text;
    }

    std::expected<DataNode, Error> array(int depth) {
        if (depth > kMaxDepth) {
            return invalidInput("The arrays nest too deeply.");
        }
        auto node = DataNode::makeArray(0, 0);
        const auto size = word(sizeof(std::uint16_t));
        const auto line = word(sizeof(std::uint16_t));
        const auto sorted = word(sizeof(std::uint16_t));
        const auto file = word();
        if (!size || !line || !sorted || !file) {
            return invalidInput(
                std::format("An array header at offset {:#x} is cut short.", offset_));
        }
        node.line = static_cast<std::uint16_t>(*line);
        node.sorted = static_cast<std::uint16_t>(*sorted);
        node.file = static_cast<std::int32_t>(*file);
        std::vector<std::uint32_t> tags((*size + kTagsPerWord - 1) / kTagsPerWord);
        for (auto &tag : tags) {
            const auto read = word();
            if (!read) {
                return std::unexpected(read.error());
            }
            tag = *read;
        }
        for (std::size_t i = 0; i < *size; ++i) {
            const auto type = static_cast<DataNode::Type>(
                (tags[i / kTagsPerWord] >> ((i % kTagsPerWord) * kTagBits)) & kTagMask);
            std::expected<DataNode, Error> child;
            switch (type) {
            case DataNode::Type::Symbol:
                child = string().transform(DataNode::makeSymbol);
                break;
            case DataNode::Type::Array:
                child = array(depth + 1);
                break;
            default:
                child = word().transform([type](std::uint32_t value) {
                    DataNode scalar;
                    scalar.type = type;
                    scalar.value = value;
                    return scalar;
                });
                break;
            }
            if (!child) {
                return std::unexpected(child.error());
            }
            node.children.push_back(std::move(*child));
        }
        return node;
    }

    [[nodiscard]] bool atEnd() const {
        return offset_ == data_.size();
    }

private:
    std::span<const std::uint8_t> data_;
    std::size_t offset_ = 0;
};

std::expected<DataFile, Error> parsePlain(std::span<const std::uint8_t> data) {
    if (data.empty() || data.front() != DataFile::kVersion) {
        return invalidInput("The file is not a version 2 compiled DataArray file.");
    }
    Reader reader(data.subspan(1));
    DataFile file;
    const auto count = reader.word();
    if (!count) {
        return std::unexpected(count.error());
    }
    for (std::uint32_t i = 0; i < *count; ++i) {
        auto name = reader.string();
        if (!name) {
            return std::unexpected(name.error());
        }
        file.files.push_back(std::move(*name));
    }
    auto root = reader.array(0);
    if (!root) {
        return std::unexpected(root.error());
    }
    if (!reader.atEnd()) {
        return invalidInput("Data follows the root array.");
    }
    file.root = std::move(*root);
    return file;
}

void putWord(std::vector<std::uint8_t> &out,
             std::uint32_t value,
             std::size_t width = sizeof(std::uint32_t)) {
    out.resize(out.size() + width);
    writeLittle(out, out.size() - width, value, width);
}

void putString(std::vector<std::uint8_t> &out, const std::string &text) {
    putWord(out, static_cast<std::uint32_t>(text.size()));
    out.insert(out.end(), text.begin(), text.end());
}

void putArray(std::vector<std::uint8_t> &out, const DataNode &node) {
    putWord(out, static_cast<std::uint32_t>(node.children.size()), sizeof(std::uint16_t));
    putWord(out, node.line, sizeof(std::uint16_t));
    putWord(out, node.sorted, sizeof(std::uint16_t));
    putWord(out, static_cast<std::uint32_t>(node.file));
    std::vector<std::uint32_t> tags((node.children.size() + kTagsPerWord - 1) / kTagsPerWord);
    for (std::size_t i = 0; i < node.children.size(); ++i) {
        tags[i / kTagsPerWord] |= static_cast<std::uint32_t>(node.children[i].type)
                                  << ((i % kTagsPerWord) * kTagBits);
    }
    for (const auto tag : tags) {
        putWord(out, tag);
    }
    for (const auto &child : node.children) {
        switch (child.type) {
        case DataNode::Type::Symbol:
            putString(out, child.symbol);
            break;
        case DataNode::Type::Array:
            putArray(out, child);
            break;
        default:
            putWord(out, child.value);
            break;
        }
    }
}

} // namespace

std::expected<DataFile, Error> DataFile::parse(std::span<const std::uint8_t> data) {
    // A plain file opens with the version byte, but the low byte of a seed may equal it, so the
    // plain form counts only when it parses to the end.
    auto plain = parsePlain(data);
    if (plain || data.size() <= kSeedSize) {
        return plain;
    }
    const auto seed = readLittle32(data, 0);
    std::vector<std::uint8_t> decrypted(data.begin() + kSeedSize, data.end());
    applyCipher(seed, decrypted);
    auto encrypted = parsePlain(decrypted);
    if (!encrypted) {
        return invalidInput(std::format("The file parses neither plain ({}) nor encrypted ({}).",
                                        plain.error().message,
                                        encrypted.error().message));
    }
    encrypted->seed = seed;
    return encrypted;
}

std::vector<std::uint8_t> DataFile::serialize() const {
    std::vector<std::uint8_t> body{kVersion};
    putWord(body, static_cast<std::uint32_t>(files.size()));
    for (const auto &name : files) {
        putString(body, name);
    }
    putArray(body, root);
    if (!seed) {
        return body;
    }
    std::vector<std::uint8_t> out;
    out.reserve(kSeedSize + body.size());
    putWord(out, *seed);
    applyCipher(*seed, body);
    std::ranges::copy(body, std::back_inserter(out));
    return out;
}

std::int32_t DataFile::fileIndex(const std::string &name) {
    if (const auto found = std::ranges::find(files, name); found != files.end()) {
        return static_cast<std::int32_t>(found - files.begin());
    }
    files.push_back(name);
    return static_cast<std::int32_t>(files.size() - 1);
}

} // namespace Tools::Ark

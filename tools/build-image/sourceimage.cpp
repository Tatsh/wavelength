#include "sourceimage.h"

#include <algorithm>
#include <format>
#include <string_view>

#include "byteorder.h"
#include "imageerror.h"

namespace Tools::BuildImage {

namespace {

constexpr std::string_view kIsoSuffix = ".iso";

// The offsets below are within the primary volume descriptor.
constexpr std::size_t kPvdVolumeSectors = 80;
constexpr std::size_t kPvdRootLba = 158;
constexpr std::size_t kPvdRootSize = 166;

// The constants below describe a directory record.
constexpr std::size_t kRecordLba = 2;
constexpr std::size_t kRecordSize = 10;
constexpr std::size_t kRecordFlags = 25;
constexpr std::size_t kRecordNameLength = 32;
constexpr std::size_t kRecordName = 33;
constexpr std::uint8_t kFlagDirectory = 0x02;
constexpr std::uint8_t kFlagMultiExtent = 0x80;

} // namespace

SourceImage::SourceImage(std::ifstream handle) : handle_(std::move(handle)) {
}

std::expected<std::unique_ptr<SourceImage>, Error>
SourceImage::open(const std::filesystem::path &path) {
    if (lowerAscii(path.extension().string()) != kIsoSuffix) {
        return discImageError(
            std::format("The disc image suffix {} is not supported.", path.extension().string()));
    }
    std::ifstream handle(path, std::ios::binary);
    if (!handle) {
        return ioError(std::format("Cannot read {}.", path.string()));
    }
    std::unique_ptr<SourceImage> image(new SourceImage(std::move(handle)));
    Sector descriptor;
    if (auto read = image->readSector(kPvdSector, descriptor); !read) {
        return std::unexpected(std::move(read.error()));
    }
    if (descriptor[0] != 1 ||
        !std::equal(kPvdMagic.begin(), kPvdMagic.end(), descriptor.begin() + 1)) {
        return discImageError(std::format("{} has no primary volume descriptor.", path.string()));
    }
    image->volumeSectors_ = readLittle32(descriptor, kPvdVolumeSectors);
    return image;
}

std::expected<LocatedFile, Error> SourceImage::find(const std::string &name) {
    Sector descriptor;
    if (auto read = readSector(kPvdSector, descriptor); !read) {
        return std::unexpected(std::move(read.error()));
    }
    struct Directory {
        std::uint32_t lba;
        std::uint32_t size;
        std::filesystem::path path;
    };
    const auto folded = upperAscii(name);
    std::vector<Directory> stack{
        {readLittle32(descriptor, kPvdRootLba), readLittle32(descriptor, kPvdRootSize), "/"}};
    std::vector<std::filesystem::path> paths;
    std::vector<LocatedFile> matches;
    while (!stack.empty()) {
        const auto directory = stack.back();
        stack.pop_back();
        const auto listing = records(directory.lba, directory.size);
        if (!listing) {
            return std::unexpected(listing.error());
        }
        for (const auto &record : *listing) {
            if (record.name == std::string_view("\0", 1) ||
                record.name == std::string_view("\1", 1)) {
                continue;
            }
            const auto bare = record.name.substr(0, record.name.find(';'));
            if (record.isDirectory) {
                stack.push_back({record.lba, record.size, directory.path / bare});
            } else if (upperAscii(bare) == folded) {
                if (record.multiExtent) {
                    return discImageError(std::format("{} spans several extents.", name));
                }
                paths.push_back(directory.path / bare);
                matches.push_back({record.lba, directory.lba, directory.size, record.size});
            }
        }
    }
    if (matches.empty()) {
        return discImageError(std::format("{} is missing from the image.", name));
    }
    if (matches.size() > 1) {
        return discImageError(ambiguousName(name, paths));
    }
    return matches.front();
}

std::expected<void, Error> SourceImage::readSector(std::uint32_t lba,
                                                   std::span<std::uint8_t, kSectorData> sector) {
    if (readSectors(lba, sector).value_or(0) != 1) {
        return pastEnd(lba);
    }
    return {};
}

std::expected<std::size_t, Error> SourceImage::readSectors(std::uint32_t lba,
                                                           std::span<std::uint8_t> sectors) {
    const auto count = sectors.size() / kSectorData;
    handle_.clear();
    handle_.seekg(static_cast<std::streamoff>(std::uint64_t{lba} * kSectorData));
    // The streams read char.
    handle_.read(reinterpret_cast<char *>(sectors.data()),
                 static_cast<std::streamsize>(count * kSectorData));
    return static_cast<std::size_t>(handle_.gcount()) / kSectorData;
}

std::uint32_t SourceImage::volumeSectors() const {
    return volumeSectors_;
}

std::expected<std::vector<SourceImage::Record>, Error> SourceImage::records(std::uint32_t lba,
                                                                            std::uint32_t size) {
    std::vector<std::uint8_t> extent(static_cast<std::size_t>(sectorsFor(size)) * kSectorData);
    for (std::uint32_t i = 0; i < sectorsFor(size); ++i) {
        if (auto read = readSector(lba + i,
                                   std::span(extent).subspan(i * kSectorData).first<kSectorData>());
            !read) {
            return std::unexpected(std::move(read.error()));
        }
    }
    std::vector<Record> found;
    std::size_t offset = 0;
    while (offset < extent.size()) {
        const auto length = extent[offset];
        if (length == 0) {
            offset = (offset / kSectorData + 1) * kSectorData;
            continue;
        }
        const auto record = std::span(extent).subspan(offset);
        if (record.size() <= kRecordNameLength ||
            kRecordName + record[kRecordNameLength] > record.size()) {
            return discImageError(std::format("Directory record at sector {} is truncated.", lba));
        }
        const auto flags = record[kRecordFlags];
        const auto name = record.subspan(kRecordName, record[kRecordNameLength]);
        found.push_back({(flags & kFlagDirectory) != 0,
                         readLittle32(record, kRecordLba),
                         (flags & kFlagMultiExtent) != 0,
                         std::string(name.begin(), name.end()),
                         readLittle32(record, kRecordSize)});
        offset += length;
    }
    return found;
}

} // namespace Tools::BuildImage

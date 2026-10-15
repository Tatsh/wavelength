#include "udftree.h"

#include <algorithm>
#include <format>
#include <span>
#include <utility>

#include "byteorder.h"
#include "imageerror.h"

namespace Tools::BuildImage {

namespace {

// Tag identifiers of ECMA-167.
constexpr std::uint16_t kTagAnchor = 2;
constexpr std::uint16_t kTagPartition = 5;
constexpr std::uint16_t kTagLogicalVolume = 6;
constexpr std::uint16_t kTagTerminating = 8;
constexpr std::uint16_t kTagIntegrity = 9;
constexpr std::uint16_t kTagFileSet = 256;
constexpr std::uint16_t kTagFileIdentifier = 257;
constexpr std::uint16_t kTagFileEntry = 261;
constexpr std::uint16_t kTagExtendedFileEntry = 266;

// The offsets below are within a descriptor tag.
constexpr std::size_t kTagLength = 16;
constexpr std::size_t kTagChecksum = 4;
constexpr std::size_t kTagCrc = 8;
constexpr std::size_t kTagCrcLength = 10;
constexpr std::size_t kTagLocation = 12;

// The offsets below are within the anchor, partition, logical volume, and file set descriptors.
constexpr std::size_t kAnchorMain = 16;
constexpr std::size_t kAnchorReserve = 24;
constexpr std::size_t kPartitionStart = 188;
constexpr std::size_t kPartitionLength = 192;
constexpr std::size_t kLogicalVolumeFileSet = 252;
constexpr std::size_t kLogicalVolumeIntegrity = 436;
constexpr std::size_t kFileSetRoot = 404;

// The offsets below are within a logical volume integrity descriptor.
constexpr std::size_t kIntegrityTime = 16;
constexpr std::size_t kIntegrityPartitions = 72;
constexpr std::size_t kIntegrityFreeSpace = 80;

// The offsets below are within a file entry or an extended file entry.
constexpr std::size_t kEntryFileType = 27;
constexpr std::size_t kEntryFlags = 34;
constexpr std::size_t kEntryInformationLength = 56;
constexpr std::size_t kExtendedObjectSize = 64;
constexpr std::uint8_t kFileTypeDirectory = 4;
constexpr std::uint16_t kAllocationTypeMask = 7;
constexpr std::uint16_t kAllocationShort = 0;
constexpr std::uint16_t kAllocationLong = 1;
constexpr std::uint16_t kAllocationEmbedded = 3;
constexpr std::size_t kShortLength = 8;
constexpr std::size_t kLongLength = 16;
constexpr std::uint32_t kExtentLengthMask = 0x3FFFFFFF;

// The offsets below are within a file identifier descriptor.
constexpr std::size_t kIdentifierCharacteristics = 18;
constexpr std::size_t kIdentifierNameLength = 19;
constexpr std::size_t kIdentifierIcbBlock = 24;
constexpr std::size_t kIdentifierUseLength = 36;
constexpr std::size_t kIdentifierFixedLength = 38;
constexpr std::uint8_t kCharacteristicParent = 0x08;

// The layout differences between a file entry and an extended file entry.
struct EntryLayout {
    std::size_t recorded;
    std::size_t modified;
    std::size_t attribute;
    std::size_t lengths;
    std::size_t descriptors;
};

constexpr EntryLayout kFileEntryLayout{64, 84, 96, 168, 176};
constexpr EntryLayout kExtendedEntryLayout{72, 92, 116, 208, 216};

// A UDF timestamp has a type and zone word, then the year and the fields down to microseconds.
constexpr std::size_t kTimeLength = 12;
constexpr std::uint16_t kTimeTypeLocal = 0x1000;
constexpr std::uint16_t kTimeZoneMask = 0xFFF;
constexpr int kTimeZoneUnspecified = -2047;
constexpr int kTimeZoneSignBit = 0x800;

std::uint64_t readLittle64(std::span<const std::uint8_t> bytes, std::size_t offset) {
    return readLittle32(bytes, offset) | (std::uint64_t{readLittle32(bytes, offset + 4)} << 32);
}

void writeLittle64(std::span<std::uint8_t> bytes, std::size_t offset, std::uint64_t value) {
    writeLittle(bytes, offset, static_cast<std::uint32_t>(value));
    writeLittle(bytes, offset + 4, static_cast<std::uint32_t>(value >> 32));
}

Seconds readTime(std::span<const std::uint8_t> bytes, std::size_t offset) {
    int zone = readLittle16(bytes, offset) & kTimeZoneMask;
    zone = (zone & kTimeZoneSignBit) != 0 ? zone - (kTimeZoneMask + 1) : zone;
    const std::chrono::year_month_day day{
        std::chrono::year{static_cast<std::int16_t>(readLittle16(bytes, offset + 2))},
        std::chrono::month{bytes[offset + 4]},
        std::chrono::day{bytes[offset + 5]}};
    return std::chrono::sys_days{day} + std::chrono::hours{bytes[offset + 6]} +
           std::chrono::minutes{bytes[offset + 7]} + std::chrono::seconds{bytes[offset + 8]} -
           std::chrono::minutes{zone == kTimeZoneUnspecified ? 0 : zone};
}

void writeTime(std::span<std::uint8_t> bytes, std::size_t offset, Seconds time) {
    const auto midnight = std::chrono::floor<std::chrono::days>(time);
    const std::chrono::year_month_day day{midnight};
    const std::chrono::hh_mm_ss clock{time - midnight};
    std::fill_n(bytes.begin() + static_cast<std::ptrdiff_t>(offset), kTimeLength, 0);
    writeLittle(bytes, offset, kTimeTypeLocal, 2);
    writeLittle(bytes, offset + 2, static_cast<std::uint32_t>(static_cast<int>(day.year())), 2);
    bytes[offset + 4] = static_cast<std::uint8_t>(static_cast<unsigned>(day.month()));
    bytes[offset + 5] = static_cast<std::uint8_t>(static_cast<unsigned>(day.day()));
    bytes[offset + 6] = static_cast<std::uint8_t>(clock.hours().count());
    bytes[offset + 7] = static_cast<std::uint8_t>(clock.minutes().count());
    bytes[offset + 8] = static_cast<std::uint8_t>(clock.seconds().count());
}

// CRC-16/CCITT with a zero initial value, as ECMA-167 7.2.6 defines the descriptor CRC.
std::uint16_t crc16(std::span<const std::uint8_t> bytes) {
    constexpr std::uint16_t kPolynomial = 0x1021;
    std::uint16_t crc = 0;
    for (const auto byte : bytes) {
        crc ^= static_cast<std::uint16_t>(byte << 8);
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000) != 0 ? static_cast<std::uint16_t>((crc << 1) ^ kPolynomial) :
                                        static_cast<std::uint16_t>(crc << 1);
        }
    }
    return crc;
}

std::uint8_t tagChecksum(std::span<const std::uint8_t> bytes) {
    std::uint8_t sum = 0;
    for (std::size_t i = 0; i < kTagLength; ++i) {
        sum = static_cast<std::uint8_t>(sum + (i == kTagChecksum ? 0 : bytes[i]));
    }
    return sum;
}

std::uint16_t tagIdentifier(std::span<const std::uint8_t> bytes) {
    return readLittle16(bytes, 0);
}

bool isTag(std::span<const std::uint8_t> bytes, std::uint16_t identifier, std::uint32_t location) {
    return tagIdentifier(bytes) == identifier && tagChecksum(bytes) == bytes[kTagChecksum] &&
           readLittle32(bytes, kTagLocation) == location;
}

void seal(Volume::Sector &descriptor) {
    const auto length = std::min<std::size_t>(readLittle16(descriptor, kTagCrcLength),
                                              descriptor.size() - kTagLength);
    writeLittle(descriptor, kTagCrc, crc16(std::span(descriptor).subspan(kTagLength, length)), 2);
    descriptor[kTagChecksum] = tagChecksum(descriptor);
}

const EntryLayout *layoutOf(std::span<const std::uint8_t> entry) {
    switch (tagIdentifier(entry)) {
    case kTagFileEntry:
        return &kFileEntryLayout;
    case kTagExtendedFileEntry:
        return &kExtendedEntryLayout;
    default:
        return nullptr;
    }
}

std::unexpected<Error> malformed(std::uint32_t lba) {
    return discImageError(std::format("The UDF descriptor at sector {} is malformed.", lba));
}

} // namespace

std::expected<std::optional<UdfTree>, Error> UdfTree::open(Volume &volume) {
    Volume::Sector anchor;
    if (!volume.readSector(kAnchorSector, anchor) || !isTag(anchor, kTagAnchor, kAnchorSector)) {
        return std::nullopt;
    }
    UdfTree tree;
    std::optional<std::uint32_t> fileSet;
    for (const auto extent : {kAnchorMain, kAnchorReserve}) {
        const auto sectors = Volume::sectorsFor(readLittle32(anchor, extent));
        const auto start = readLittle32(anchor, extent + 4);
        for (std::uint32_t lba = start; lba < start + sectors; ++lba) {
            Volume::Sector descriptor;
            if (auto read = volume.readSector(lba, descriptor); !read) {
                return std::unexpected(std::move(read.error()));
            }
            const auto identifier = tagIdentifier(descriptor);
            if (identifier == kTagTerminating) {
                break;
            }
            if (identifier == kTagPartition) {
                tree.partitionStart_ = readLittle32(descriptor, kPartitionStart);
                tree.partitionDescriptors_.push_back(lba);
            } else if (identifier == kTagLogicalVolume) {
                fileSet = readLittle32(descriptor, kLogicalVolumeFileSet);
                if (readLittle32(descriptor, kLogicalVolumeIntegrity - 4) != 0) {
                    tree.integrityDescriptor_ = readLittle32(descriptor, kLogicalVolumeIntegrity);
                }
            }
        }
    }
    if (tree.partitionDescriptors_.empty() || !fileSet) {
        return malformed(kAnchorSector);
    }
    if (tree.integrityDescriptor_) {
        Volume::Sector integrity;
        if (!volume.readSector(*tree.integrityDescriptor_, integrity) ||
            !isTag(integrity, kTagIntegrity, *tree.integrityDescriptor_)) {
            tree.integrityDescriptor_.reset();
        }
    }
    Volume::Sector descriptor;
    const auto fileSetLba = tree.partitionStart_ + *fileSet;
    if (auto read = volume.readSector(fileSetLba, descriptor); !read) {
        return std::unexpected(std::move(read.error()));
    }
    if (!isTag(descriptor, kTagFileSet, *fileSet)) {
        return malformed(fileSetLba);
    }
    if (auto walked = tree.walk(volume, readLittle32(descriptor, kFileSetRoot)); !walked) {
        return std::unexpected(std::move(walked.error()));
    }
    return tree;
}

std::expected<void, Error> UdfTree::walk(Volume &volume, std::uint32_t block) {
    std::vector<std::vector<std::uint32_t>> stack{{partitionStart_ + block}};
    while (!stack.empty()) {
        const auto chain = std::move(stack.back());
        stack.pop_back();
        Volume::Sector entry;
        if (auto read = volume.readSector(chain.front(), entry); !read) {
            return read;
        }
        const auto data = directoryData(volume, entry);
        if (!data) {
            return std::unexpected(data.error());
        }
        const std::span<const std::uint8_t> bytes = *data;
        for (std::size_t offset = 0; offset + kIdentifierFixedLength <= bytes.size();) {
            if (tagIdentifier(bytes.subspan(offset)) != kTagFileIdentifier) {
                break;
            }
            const auto characteristics = bytes[offset + kIdentifierCharacteristics];
            const auto lba = partitionStart_ + readLittle32(bytes, offset + kIdentifierIcbBlock);
            offset += (kIdentifierFixedLength + readLittle16(bytes, offset + kIdentifierUseLength) +
                       bytes[offset + kIdentifierNameLength] + 3) &
                      ~std::size_t{3};
            if ((characteristics & kCharacteristicParent) != 0) {
                continue;
            }
            Volume::Sector child;
            if (auto read = volume.readSector(lba, child); !read) {
                return read;
            }
            const auto *layout = layoutOf(child);
            if (layout == nullptr) {
                return malformed(lba);
            }
            if (child[kEntryFileType] == kFileTypeDirectory) {
                std::vector<std::uint32_t> nested{lba};
                nested.insert(nested.end(), chain.begin(), chain.end());
                stack.push_back(std::move(nested));
                continue;
            }
            const auto type = readLittle16(child, kEntryFlags) & kAllocationTypeMask;
            if (type != kAllocationShort && type != kAllocationLong) {
                continue;
            }
            const auto step = type == kAllocationShort ? kShortLength : kLongLength;
            const auto start = layout->descriptors + readLittle32(child, layout->lengths);
            const auto length = readLittle32(child, layout->lengths + 4);
            if (length == 0 || start + step > child.size()) {
                continue;
            }
            files_.insert_or_assign(partitionStart_ + readLittle32(child, start + 4),
                                    File{lba,
                                         chain,
                                         length / step,
                                         readTime(child, layout->modified),
                                         readLittle64(child, kEntryInformationLength)});
        }
    }
    return {};
}

std::expected<std::vector<std::uint8_t>, Error>
UdfTree::directoryData(Volume &volume, const Volume::Sector &entry) {
    const auto *layout = layoutOf(entry);
    if (layout == nullptr) {
        return malformed(readLittle32(entry, kTagLocation) + partitionStart_);
    }
    const auto start = layout->descriptors + readLittle32(entry, layout->lengths);
    const auto length = readLittle32(entry, layout->lengths + 4);
    if (start + length > entry.size()) {
        return malformed(readLittle32(entry, kTagLocation) + partitionStart_);
    }
    const auto descriptors = std::span(entry).subspan(start, length);
    const auto type = readLittle16(entry, kEntryFlags) & kAllocationTypeMask;
    if (type == kAllocationEmbedded) {
        return std::vector<std::uint8_t>(descriptors.begin(), descriptors.end());
    }
    const auto step = type == kAllocationShort ? kShortLength : kLongLength;
    std::vector<std::uint8_t> data;
    for (std::size_t offset = 0; offset + step <= descriptors.size(); offset += step) {
        const auto extent = readLittle32(descriptors, offset) & kExtentLengthMask;
        const auto block = readLittle32(descriptors, offset + 4);
        for (std::uint32_t i = 0; i < Volume::sectorsFor(extent); ++i) {
            Volume::Sector sector;
            if (auto read = volume.readSector(partitionStart_ + block + i, sector); !read) {
                return std::unexpected(std::move(read.error()));
            }
            data.insert(data.end(), sector.begin(), sector.end());
        }
    }
    data.resize(std::min<std::size_t>(data.size(), readLittle64(entry, kEntryInformationLength)));
    return data;
}

std::expected<UdfTree::File, Error> UdfTree::find(std::uint32_t lba) const {
    const auto found = files_.find(lba);
    if (found == files_.end()) {
        return discImageError(std::format("The UDF tree has no file at sector {}.", lba));
    }
    if (found->second.extents != 1) {
        return discImageError(std::format(
            "The UDF file at sector {} has {} extents, not one.", lba, found->second.extents));
    }
    return found->second;
}

std::uint32_t UdfTree::partitionStart() const {
    return partitionStart_;
}

const std::vector<std::uint32_t> &UdfTree::partitionDescriptors() const {
    return partitionDescriptors_;
}

std::optional<std::uint32_t> UdfTree::integrityDescriptor() const {
    return integrityDescriptor_;
}

void UdfTree::setFile(Volume::Sector &entry,
                      std::uint32_t size,
                      std::uint32_t block,
                      Seconds modified) {
    const auto *layout = layoutOf(entry);
    const auto start = layout->descriptors + readLittle32(entry, layout->lengths);
    writeLittle64(entry, kEntryInformationLength, size);
    if (layout == &kExtendedEntryLayout) {
        writeLittle64(entry, kExtendedObjectSize, size);
    }
    writeLittle64(entry, layout->recorded, Volume::sectorsFor(size));
    const auto type = readLittle32(entry, start) & ~kExtentLengthMask;
    writeLittle(entry, start, type | size);
    writeLittle(entry, start + 4, block);
    writeTime(entry, layout->modified, modified);
    writeTime(entry, layout->attribute, modified);
    seal(entry);
}

void UdfTree::raiseModified(Volume::Sector &entry, Seconds time) {
    const auto *layout = layoutOf(entry);
    if (readTime(entry, layout->modified) < time) {
        writeTime(entry, layout->modified, time);
        seal(entry);
    }
}

void UdfTree::raiseIntegrity(Volume::Sector &descriptor,
                             Seconds time,
                             std::uint32_t partitionLength) {
    if (readTime(descriptor, kIntegrityTime) < time) {
        writeTime(descriptor, kIntegrityTime, time);
    }
    // The size table follows the free space table, one word per partition.
    const auto partitions = readLittle32(descriptor, kIntegrityPartitions);
    if (partitions == 1) {
        writeLittle(descriptor, kIntegrityFreeSpace + 4, partitionLength);
    }
    seal(descriptor);
}

std::uint32_t UdfTree::partitionLength(const Volume::Sector &descriptor) {
    return readLittle32(descriptor, kPartitionLength);
}

void UdfTree::setPartitionLength(Volume::Sector &descriptor, std::uint32_t length) {
    writeLittle(descriptor, kPartitionLength, length);
    seal(descriptor);
}

void UdfTree::relocate(Volume::Sector &descriptor, std::uint32_t location) {
    writeLittle(descriptor, kTagLocation, location);
    seal(descriptor);
}

} // namespace Tools::BuildImage

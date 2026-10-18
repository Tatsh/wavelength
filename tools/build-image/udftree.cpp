#include "udftree.h"

#include <algorithm>
#include <format>
#include <span>
#include <utility>

#include "byteorder.h"
#include "imageerror.h"
#include "udfformat.h"

namespace Tools::BuildImage {

using Common::discImageError;
using Common::Error;
using Common::readLittle16;
using Common::readLittle32;
using Common::Seconds;
using Common::Volume;
using Common::writeLittle;

namespace {

using namespace Udf;

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

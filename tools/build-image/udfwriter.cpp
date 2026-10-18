#include "udfwriter.h"

#include <algorithm>
#include <format>
#include <string_view>

#include "udfformat.h"
#include "udftree.h"

namespace Tools::BuildImage {

using Common::Seconds;
using Common::Volume;
using Common::writeLittle;

namespace {

using namespace Udf;

constexpr std::size_t kSectorData = Volume::kSectorData;

// The sectors of the volume structures.
constexpr std::uint32_t kRecognitionSector = 18;
constexpr std::uint32_t kMainSequenceSector = 32;
constexpr std::uint32_t kReserveSequenceSector = 48;
constexpr std::uint32_t kIntegritySector = 64;
constexpr std::uint32_t kSequenceExtent = 16 * kSectorData;
constexpr std::uint32_t kIntegrityExtent = 2 * kSectorData;

// Tag identifiers the reader does not need.
constexpr std::uint16_t kTagPrimaryVolume = 1;
constexpr std::uint16_t kTagImplementationUse = 4;
constexpr std::uint16_t kTagUnallocatedSpace = 7;
constexpr std::uint16_t kDescriptorVersion = 2;
constexpr std::uint16_t kSectorCrcLength = kSectorData - kTagLength;

// Recognition sequence descriptors.
constexpr std::string_view kBeginExtended = "BEA01";
constexpr std::string_view kNsr = "NSR02";
constexpr std::string_view kTerminateExtended = "TEA01";

// Entity identifiers and their suffixes. The UDF revision is 1.02.
constexpr std::size_t kRegidLength = 32;
constexpr std::size_t kRegidIdentifier = 1;
constexpr std::size_t kRegidSuffix = 24;
constexpr std::uint16_t kUdfRevision = 0x0102;
constexpr std::uint8_t kDomainWriteProtected = 3;
constexpr std::string_view kDomainIdentifier = "*OSTA UDF Compliant";
constexpr std::string_view kLogicalVolumeInfo = "*UDF LV Info";
constexpr std::string_view kNsrContents = "+NSR02";
constexpr std::string_view kImplementation = "*Wavelength build-image";
constexpr std::string_view kApplication = "PLAYSTATION";
constexpr std::string_view kFileSetName = "PLAYSTATION2 DVD-ROM FILE SET";

// A character set specification of OSTA compressed Unicode, and the 8-bit compression of a
// d-string.
constexpr std::size_t kCharspecLength = 64;
constexpr std::string_view kOstaCharset = "OSTA Compressed Unicode";
constexpr std::uint8_t kCompression8 = 8;

// The offsets below are within the primary volume descriptor.
constexpr std::size_t kPrimarySequence = 16;
constexpr std::size_t kPrimaryIdentifier = 24;
constexpr std::size_t kPrimaryIdentifierLength = 32;
constexpr std::size_t kPrimaryVolumeSequence = 56;
constexpr std::size_t kPrimaryMaxVolumeSequence = 58;
constexpr std::size_t kPrimaryInterchange = 60;
constexpr std::size_t kPrimaryMaxInterchange = 62;
constexpr std::size_t kPrimaryCharsets = 64;
constexpr std::size_t kPrimaryMaxCharsets = 68;
constexpr std::size_t kPrimaryVolumeSet = 72;
constexpr std::size_t kPrimaryVolumeSetLength = 128;
constexpr std::size_t kPrimaryCharset = 200;
constexpr std::size_t kPrimaryExplanatoryCharset = 264;
constexpr std::size_t kPrimaryApplication = 344;
constexpr std::size_t kPrimaryRecorded = 376;
constexpr std::size_t kPrimaryImplementation = 388;
constexpr std::uint16_t kInterchangeLevel = 2;

// The offsets below are within the implementation use volume descriptor.
constexpr std::size_t kUseIdentifier = 20;
constexpr std::size_t kUseCharset = 52;
constexpr std::size_t kUseVolumeIdentifier = 116;
constexpr std::size_t kUseVolumeIdentifierLength = 128;
constexpr std::size_t kUseImplementation = 352;

// The offsets below are within the partition descriptor.
constexpr std::size_t kPartitionSequence = 16;
constexpr std::size_t kPartitionContents = 24;
constexpr std::size_t kPartitionAccess = 184;
constexpr std::size_t kPartitionImplementation = 196;
constexpr std::uint32_t kAccessReadOnly = 1;

// The offsets below are within the logical volume descriptor.
constexpr std::size_t kVolumeSequence = 16;
constexpr std::size_t kVolumeCharset = 20;
constexpr std::size_t kVolumeIdentifier = 84;
constexpr std::size_t kVolumeIdentifierLength = 128;
constexpr std::size_t kVolumeBlockSize = 212;
constexpr std::size_t kVolumeDomain = 216;
constexpr std::size_t kVolumeFileSetLength = 248;
constexpr std::size_t kVolumeMapTableLength = 264;
constexpr std::size_t kVolumeMapCount = 268;
constexpr std::size_t kVolumeImplementation = 272;
constexpr std::size_t kVolumeIntegrityLength = 432;
constexpr std::size_t kVolumeMaps = 440;
// A type 1 partition map has the type, its length, the volume sequence number, and the partition.
constexpr std::uint8_t kMapType = 1;
constexpr std::uint8_t kMapLength = 6;

// The offsets below are within the unallocated space descriptor and the integrity descriptor.
constexpr std::size_t kUnallocatedSequence = 16;
constexpr std::size_t kIntegrityType = 28;
constexpr std::size_t kIntegrityUniqueId = 40;
constexpr std::size_t kIntegrityUseLength = 76;
constexpr std::uint32_t kIntegrityClosed = 1;
constexpr std::uint32_t kIntegrityUseBytes = 46;
constexpr std::size_t kIntegrityUseFiles = kRegidLength;
constexpr std::size_t kIntegrityUseDirectories = kRegidLength + 4;
constexpr std::size_t kIntegrityUseRevisions = kRegidLength + 8;
constexpr int kIntegrityRevisionCount = 3;

// The offsets below are within the file set descriptor.
constexpr std::size_t kFileSetRecorded = 16;
constexpr std::size_t kFileSetInterchange = 28;
constexpr std::size_t kFileSetMaxInterchange = 30;
constexpr std::size_t kFileSetCharsets = 32;
constexpr std::size_t kFileSetMaxCharsets = 36;
constexpr std::size_t kFileSetVolumeCharset = 48;
constexpr std::size_t kFileSetVolumeIdentifier = 112;
constexpr std::size_t kFileSetCharset = 240;
constexpr std::size_t kFileSetIdentifier = 304;
constexpr std::size_t kFileSetIdentifierLength = 32;
constexpr std::size_t kFileSetCopyright = 336;
constexpr std::size_t kFileSetAbstract = 368;
constexpr std::size_t kFileSetRootLength = 400;
constexpr std::size_t kFileSetDomain = 416;
constexpr std::uint16_t kFileSetInterchangeLevel = 3;

// The offsets below are within a file entry, after its tag.
constexpr std::size_t kEntryStrategy = 20;
constexpr std::size_t kEntryMaxEntries = 24;
constexpr std::size_t kEntryUid = 36;
constexpr std::size_t kEntryGid = 40;
constexpr std::size_t kEntryPermissions = 44;
constexpr std::size_t kEntryLinks = 48;
constexpr std::size_t kEntryAccessed = 72;
constexpr std::size_t kEntryCheckpoint = 108;
constexpr std::size_t kEntryImplementation = 128;
constexpr std::size_t kEntryUniqueId = 160;
constexpr std::uint16_t kStrategyDirect = 4;
constexpr std::uint8_t kFileTypeFile = 5;
constexpr std::uint32_t kNoOwner = 0xFFFFFFFF;
// Read and execute for every class of user, as on the discs Sony mastered.
constexpr std::uint32_t kPermissions = 0x14A5;
// Not relocatable, archived, contiguous, and a system file, with short allocation descriptors.
constexpr std::uint16_t kEntryFlagValue = 0x0630;
// The largest extent a short allocation descriptor records, in whole sectors.
constexpr std::uint64_t kMaxExtent = (kExtentLengthMask / kSectorData) * kSectorData;
// Unique identifiers 1 to 15 are reserved, and the root has 0.
constexpr std::uint64_t kFirstUniqueId = 16;

// The characteristics of a file identifier descriptor.
constexpr std::uint8_t kCharacteristicDirectory = 0x02;
constexpr std::size_t kIdentifierAlignment = 4;

void putText(Volume::Sector &sector, std::size_t offset, std::string_view text) {
    std::ranges::copy(text, sector.begin() + static_cast<std::ptrdiff_t>(offset));
}

void putRegid(Volume::Sector &sector,
              std::size_t offset,
              std::string_view identifier,
              std::uint8_t domainFlags = 0) {
    putText(sector, offset + kRegidIdentifier, identifier);
    if (identifier.starts_with('*') && identifier != kImplementation) {
        writeLittle(sector, offset + kRegidSuffix, kUdfRevision, 2);
        sector[offset + kRegidSuffix + 2] = domainFlags;
    }
}

void putCharspec(Volume::Sector &sector, std::size_t offset) {
    putText(sector, offset + 1, kOstaCharset);
}

// The last byte of a d-string is the length of the used part, the compression byte included.
void putDstring(Volume::Sector &sector,
                std::size_t offset,
                std::size_t length,
                std::string_view text) {
    sector[offset] = kCompression8;
    putText(sector, offset + 1, text.substr(0, length - 2));
    sector[offset + length - 1] = static_cast<std::uint8_t>(1 + std::min(text.size(), length - 2));
}

Volume::Sector &descriptor(std::map<std::uint32_t, Volume::Sector> &sectors,
                           std::uint32_t lba,
                           std::uint16_t identifier,
                           std::uint32_t location) {
    auto &sector = sectors[lba];
    sector.fill(0);
    writeLittle(sector, 0, identifier, 2);
    writeLittle(sector, 2, kDescriptorVersion, 2);
    writeLittle(sector, kTagCrcLength, kSectorCrcLength, 2);
    writeLittle(sector, kTagLocation, location);
    return sector;
}

std::size_t identifierLength(const UdfNode &node, bool parent) {
    const auto nameBytes = parent ? 0 : 1 + node.name.size();
    return (kIdentifierFixedLength + nameBytes + kIdentifierAlignment - 1) &
           ~(kIdentifierAlignment - 1);
}

void writeSequence(std::map<std::uint32_t, Volume::Sector> &sectors,
                   std::uint32_t start,
                   std::uint32_t partitionStart,
                   std::uint32_t partitionLength,
                   Seconds recorded,
                   std::string_view volumeSet) {
    std::uint32_t sequence = 0;
    auto *sector = &descriptor(sectors, start, kTagPrimaryVolume, start);
    writeLittle(*sector, kPrimarySequence, sequence++);
    putDstring(*sector, kPrimaryIdentifier, kPrimaryIdentifierLength, "");
    writeLittle(*sector, kPrimaryVolumeSequence, 1, 2);
    writeLittle(*sector, kPrimaryMaxVolumeSequence, 1, 2);
    writeLittle(*sector, kPrimaryInterchange, kInterchangeLevel, 2);
    writeLittle(*sector, kPrimaryMaxInterchange, kInterchangeLevel, 2);
    writeLittle(*sector, kPrimaryCharsets, 1);
    writeLittle(*sector, kPrimaryMaxCharsets, 1);
    putDstring(*sector, kPrimaryVolumeSet, kPrimaryVolumeSetLength, volumeSet);
    putCharspec(*sector, kPrimaryCharset);
    putCharspec(*sector, kPrimaryExplanatoryCharset);
    putRegid(*sector, kPrimaryApplication, kApplication);
    writeTime(*sector, kPrimaryRecorded, recorded);
    putRegid(*sector, kPrimaryImplementation, kImplementation);
    seal(*sector);

    sector = &descriptor(sectors, start + 1, kTagImplementationUse, start + 1);
    writeLittle(*sector, kPrimarySequence, sequence++);
    putRegid(*sector, kUseIdentifier, kLogicalVolumeInfo);
    putCharspec(*sector, kUseCharset);
    putDstring(*sector, kUseVolumeIdentifier, kUseVolumeIdentifierLength, "");
    putRegid(*sector, kUseImplementation, kImplementation);
    seal(*sector);

    sector = &descriptor(sectors, start + 2, kTagPartition, start + 2);
    writeLittle(*sector, kPartitionSequence, sequence++);
    putRegid(*sector, kPartitionContents, kNsrContents);
    writeLittle(*sector, kPartitionAccess, kAccessReadOnly);
    writeLittle(*sector, kPartitionStart, partitionStart);
    writeLittle(*sector, kPartitionLength, partitionLength);
    putRegid(*sector, kPartitionImplementation, kImplementation);
    seal(*sector);

    sector = &descriptor(sectors, start + 3, kTagLogicalVolume, start + 3);
    writeLittle(*sector, kVolumeSequence, sequence++);
    putCharspec(*sector, kVolumeCharset);
    putDstring(*sector, kVolumeIdentifier, kVolumeIdentifierLength, "");
    writeLittle(*sector, kVolumeBlockSize, kSectorData);
    putRegid(*sector, kVolumeDomain, kDomainIdentifier, kDomainWriteProtected);
    writeLittle(*sector, kVolumeFileSetLength, UdfWriter::kPartitionHeaderSectors * kSectorData);
    writeLittle(*sector, kVolumeMapTableLength, kMapLength);
    writeLittle(*sector, kVolumeMapCount, 1);
    putRegid(*sector, kVolumeImplementation, kImplementation);
    writeLittle(*sector, kVolumeIntegrityLength, kIntegrityExtent);
    writeLittle(*sector, kLogicalVolumeIntegrity, kIntegritySector);
    (*sector)[kVolumeMaps] = kMapType;
    (*sector)[kVolumeMaps + 1] = kMapLength;
    writeLittle(*sector, kVolumeMaps + 2, 1, 2);
    seal(*sector);

    sector = &descriptor(sectors, start + 4, kTagUnallocatedSpace, start + 4);
    writeLittle(*sector, kUnallocatedSequence, sequence);
    seal(*sector);

    seal(descriptor(sectors, start + 5, kTagTerminating, start + 5));
}

void writeEntry(Volume::Sector &sector,
                const std::vector<UdfNode> &nodes,
                std::size_t index,
                std::uint32_t partitionStart) {
    const auto &node = nodes[index];
    const auto location = node.entryLba - partitionStart;
    sector.fill(0);
    writeLittle(sector, 0, kTagFileEntry, 2);
    writeLittle(sector, 2, kDescriptorVersion, 2);
    writeLittle(sector, kTagLocation, location);
    writeLittle(sector, kEntryStrategy, kStrategyDirect, 2);
    writeLittle(sector, kEntryMaxEntries, 1, 2);
    sector[kEntryFileType] = node.directory ? kFileTypeDirectory : kFileTypeFile;
    writeLittle(sector, kEntryFlags, kEntryFlagValue, 2);
    writeLittle(sector, kEntryUid, kNoOwner);
    writeLittle(sector, kEntryGid, kNoOwner);
    writeLittle(sector, kEntryPermissions, kPermissions);
    // A directory is linked from its parent and from the parent entry of each subdirectory.
    const auto links = 1 + std::ranges::count_if(node.children, [&nodes](std::size_t child) {
                           return nodes[child].directory;
                       });
    writeLittle(sector, kEntryLinks, static_cast<std::uint32_t>(links), 2);
    writeLittle64(sector, kEntryInformationLength, node.size);
    writeLittle64(sector, kFileEntryLayout.recorded, Volume::sectorsFor(node.size));
    writeTime(sector, kEntryAccessed, node.modified);
    writeTime(sector, kFileEntryLayout.modified, node.modified);
    writeTime(sector, kFileEntryLayout.attribute, node.modified);
    writeLittle(sector, kEntryCheckpoint, 1);
    putRegid(sector, kEntryImplementation, kImplementation);
    writeLittle64(sector, kEntryUniqueId, index == 0 ? 0 : kFirstUniqueId + index - 1);
    std::size_t descriptors = 0;
    auto block = node.lba - partitionStart;
    for (std::uint64_t remaining = node.size; remaining > 0 || descriptors == 0;) {
        const auto length = std::min(remaining, kMaxExtent);
        const auto offset = kFileEntryLayout.descriptors + descriptors * kShortLength;
        writeLittle(sector, offset, static_cast<std::uint32_t>(length));
        writeLittle(sector, offset + 4, block);
        block += static_cast<std::uint32_t>(length / kSectorData);
        remaining -= length;
        ++descriptors;
    }
    writeLittle(sector,
                kFileEntryLayout.lengths + 4,
                static_cast<std::uint32_t>(descriptors * kShortLength));
    writeLittle(sector,
                kTagCrcLength,
                static_cast<std::uint32_t>(kFileEntryLayout.descriptors +
                                           descriptors * kShortLength - kTagLength),
                2);
    seal(sector);
}

std::vector<std::uint8_t> directoryRecords(const std::vector<UdfNode> &nodes,
                                           std::size_t index,
                                           std::uint32_t partitionStart) {
    const auto &directory = nodes[index];
    std::vector<std::uint8_t> data;
    const auto add = [&](const UdfNode &target, std::uint8_t characteristics, bool parent) {
        const auto length = identifierLength(target, parent);
        const auto offset = data.size();
        data.resize(offset + length);
        const auto record = std::span(data).subspan(offset);
        const auto block =
            directory.lba - partitionStart + static_cast<std::uint32_t>(offset / kSectorData);
        writeLittle(record, 0, kTagFileIdentifier, 2);
        writeLittle(record, 2, kDescriptorVersion, 2);
        writeLittle(record, kTagCrcLength, static_cast<std::uint32_t>(length - kTagLength), 2);
        writeLittle(record, kTagLocation, block);
        writeLittle(record, kTagLength, 1, 2);
        record[kIdentifierCharacteristics] = characteristics;
        record[kIdentifierNameLength] =
            parent ? 0 : static_cast<std::uint8_t>(1 + target.name.size());
        writeLittle(record, kIdentifierIcbBlock - 4, kSectorData);
        writeLittle(record, kIdentifierIcbBlock, target.entryLba - partitionStart);
        if (!parent) {
            record[kIdentifierFixedLength] = kCompression8;
            std::ranges::copy(target.name, record.begin() + kIdentifierFixedLength + 1);
        }
        writeLittle(record, kTagCrc, crc16(record.subspan(kTagLength, length - kTagLength)), 2);
        record[kTagChecksum] = tagChecksum(record);
    };
    add(nodes[directory.parent], kCharacteristicDirectory | kCharacteristicParent, true);
    for (const auto child : directory.children) {
        add(nodes[child], nodes[child].directory ? kCharacteristicDirectory : 0, false);
    }
    return data;
}

} // namespace

std::uint64_t UdfWriter::directorySize(const std::vector<UdfNode> &nodes, std::size_t directory) {
    std::uint64_t size = identifierLength(nodes[directory], true);
    for (const auto child : nodes[directory].children) {
        size += identifierLength(nodes[child], false);
    }
    return size;
}

void UdfWriter::write(const std::vector<UdfNode> &nodes,
                      std::uint32_t partitionStart,
                      std::uint32_t volumeSectors,
                      Seconds recorded,
                      std::map<std::uint32_t, Volume::Sector> &sectors) {
    const auto partitionLength = volumeSectors - 1 - partitionStart;
    for (std::uint32_t i = 0; const auto identifier : {kBeginExtended, kNsr, kTerminateExtended}) {
        auto &sector = sectors[kRecognitionSector + i++];
        sector.fill(0);
        putText(sector, 1, identifier);
        sector[6] = 1;
    }
    // The first 16 characters of a volume set identifier are unique, a time in hexadecimal.
    const auto volumeSet = std::format("{:016X}", recorded.time_since_epoch().count());
    writeSequence(
        sectors, kMainSequenceSector, partitionStart, partitionLength, recorded, volumeSet);
    writeSequence(
        sectors, kReserveSequenceSector, partitionStart, partitionLength, recorded, volumeSet);

    const auto directories = std::ranges::count_if(nodes, &UdfNode::directory);
    auto *sector = &descriptor(sectors, kIntegritySector, kTagIntegrity, kIntegritySector);
    writeTime(*sector, kIntegrityTime, recorded);
    writeLittle(*sector, kIntegrityType, kIntegrityClosed);
    writeLittle64(*sector, kIntegrityUniqueId, kFirstUniqueId + nodes.size());
    writeLittle(*sector, kIntegrityPartitions, 1);
    writeLittle(*sector, kIntegrityUseLength, kIntegrityUseBytes);
    writeLittle(*sector, kIntegrityFreeSpace, 0);
    writeLittle(*sector, kIntegrityFreeSpace + 4, partitionLength);
    const auto use = kIntegrityFreeSpace + 8;
    putRegid(*sector, use, kImplementation);
    writeLittle(
        *sector, use + kIntegrityUseFiles, static_cast<std::uint32_t>(nodes.size() - directories));
    writeLittle(*sector, use + kIntegrityUseDirectories, static_cast<std::uint32_t>(directories));
    for (int i = 0; i < kIntegrityRevisionCount; ++i) {
        writeLittle(*sector, use + kIntegrityUseRevisions + (2 * i), kUdfRevision, 2);
    }
    seal(*sector);
    seal(descriptor(sectors, kIntegritySector + 1, kTagTerminating, kIntegritySector + 1));

    for (const auto lba : {std::uint32_t{UdfTree::kAnchorSector}, volumeSectors - 1}) {
        sector = &descriptor(sectors, lba, kTagAnchor, lba);
        writeLittle(*sector, kAnchorMain, kSequenceExtent);
        writeLittle(*sector, kAnchorMain + 4, kMainSequenceSector);
        writeLittle(*sector, kAnchorReserve, kSequenceExtent);
        writeLittle(*sector, kAnchorReserve + 4, kReserveSequenceSector);
        seal(*sector);
    }

    sector = &descriptor(sectors, partitionStart, kTagFileSet, 0);
    writeTime(*sector, kFileSetRecorded, recorded);
    writeLittle(*sector, kFileSetInterchange, kFileSetInterchangeLevel, 2);
    writeLittle(*sector, kFileSetMaxInterchange, kFileSetInterchangeLevel, 2);
    writeLittle(*sector, kFileSetCharsets, 1);
    writeLittle(*sector, kFileSetMaxCharsets, 1);
    putCharspec(*sector, kFileSetVolumeCharset);
    putDstring(*sector, kFileSetVolumeIdentifier, kVolumeIdentifierLength, "");
    putCharspec(*sector, kFileSetCharset);
    putDstring(*sector, kFileSetIdentifier, kFileSetIdentifierLength, kFileSetName);
    writeLittle(*sector, kFileSetRootLength, kSectorData);
    writeLittle(*sector, kFileSetRoot, nodes.front().entryLba - partitionStart);
    putRegid(*sector, kFileSetDomain, kDomainIdentifier, kDomainWriteProtected);
    seal(*sector);
    seal(descriptor(sectors, partitionStart + 1, kTagTerminating, 1));

    for (std::size_t i = 0; i < nodes.size(); ++i) {
        writeEntry(sectors[nodes[i].entryLba], nodes, i, partitionStart);
        if (!nodes[i].directory) {
            continue;
        }
        const auto data = directoryRecords(nodes, i, partitionStart);
        for (std::uint32_t s = 0; s < Volume::sectorsFor(data.size()); ++s) {
            auto &block = sectors[nodes[i].lba + s];
            block.fill(0);
            const auto start = std::size_t{s} * kSectorData;
            std::copy_n(data.begin() + static_cast<std::ptrdiff_t>(start),
                        std::min(kSectorData, data.size() - start),
                        block.begin());
        }
    }
}

} // namespace Tools::BuildImage

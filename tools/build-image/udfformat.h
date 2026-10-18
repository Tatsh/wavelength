#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <span>

#include "byteorder.h"
#include "isodate.h"
#include "volume.h"

/** The descriptor layouts of ECMA-167 and UDF that the UDF reader and writer share. */
namespace Tools::BuildImage::Udf {

using Common::readLittle16;
using Common::readLittle32;
using Common::Seconds;
using Common::Volume;
using Common::writeLittle;

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

inline std::uint64_t readLittle64(std::span<const std::uint8_t> bytes, std::size_t offset) {
    return readLittle32(bytes, offset) | (std::uint64_t{readLittle32(bytes, offset + 4)} << 32);
}

inline void writeLittle64(std::span<std::uint8_t> bytes, std::size_t offset, std::uint64_t value) {
    writeLittle(bytes, offset, static_cast<std::uint32_t>(value));
    writeLittle(bytes, offset + 4, static_cast<std::uint32_t>(value >> 32));
}

inline Seconds readTime(std::span<const std::uint8_t> bytes, std::size_t offset) {
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

inline void writeTime(std::span<std::uint8_t> bytes, std::size_t offset, Seconds time) {
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
inline std::uint16_t crc16(std::span<const std::uint8_t> bytes) {
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

inline std::uint8_t tagChecksum(std::span<const std::uint8_t> bytes) {
    std::uint8_t sum = 0;
    for (std::size_t i = 0; i < kTagLength; ++i) {
        sum = static_cast<std::uint8_t>(sum + (i == kTagChecksum ? 0 : bytes[i]));
    }
    return sum;
}

inline std::uint16_t tagIdentifier(std::span<const std::uint8_t> bytes) {
    return readLittle16(bytes, 0);
}

inline bool
isTag(std::span<const std::uint8_t> bytes, std::uint16_t identifier, std::uint32_t location) {
    return tagIdentifier(bytes) == identifier && tagChecksum(bytes) == bytes[kTagChecksum] &&
           readLittle32(bytes, kTagLocation) == location;
}

inline void seal(Volume::Sector &descriptor) {
    const auto length = std::min<std::size_t>(readLittle16(descriptor, kTagCrcLength),
                                              descriptor.size() - kTagLength);
    writeLittle(descriptor, kTagCrc, crc16(std::span(descriptor).subspan(kTagLength, length)), 2);
    descriptor[kTagChecksum] = tagChecksum(descriptor);
}

} // namespace Tools::BuildImage::Udf

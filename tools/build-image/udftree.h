#pragma once

#include <cstdint>
#include <expected>
#include <map>
#include <optional>
#include <vector>

#include "isodate.h"
#include "volume.h"

namespace Tools::BuildImage {

/**
 * The UDF bridge file system of a DVD image, beside its ISO9660 tree.
 *
 * Reading follows the anchor volume descriptor pointer at sector 256 to the partition and the
 * file set, then walks every directory from the root. Each file is found by the first sector of its
 * data, which the ISO9660 directory record of the same file also gives. The static editors change
 * one descriptor sector and recompute its descriptor CRC and tag checksum.
 */
class UdfTree {
public:
    /** Sector of the first anchor volume descriptor pointer. */
    static constexpr std::uint32_t kAnchorSector = 256;

    /** A file of the UDF tree. */
    struct File {
        std::uint32_t entry = 0; /*!< Sector of the file entry. */
        /** Sectors of the file entries of the containing directory, then each ancestor up to the
         * root directory. */
        std::vector<std::uint32_t> directories;
        std::size_t extents = 0; /*!< Number of allocation descriptors. */
        Seconds modified;        /*!< Modification time, in UTC. */
        std::uint64_t size = 0;  /*!< Information length in bytes. */
    };

    /**
     * Read the UDF structures of a volume.
     *
     * @param volume Volume to read.
     * @return The tree, nothing when sector 256 has no anchor volume descriptor pointer, or an
     * error when a descriptor is missing or malformed.
     */
    static std::expected<std::optional<UdfTree>, Error> open(Volume &volume);

    /**
     * Locate the file whose data starts at a sector.
     *
     * @param lba First data sector.
     * @return The file, or an error when no file starts there or the file has several extents.
     */
    [[nodiscard]] std::expected<File, Error> find(std::uint32_t lba) const;

    /**
     * Report the first sector of the partition.
     *
     * @return Sector of logical block zero.
     */
    [[nodiscard]] std::uint32_t partitionStart() const;

    /**
     * List the sectors of the partition descriptors in both volume descriptor sequences.
     *
     * @return Sectors of the partition descriptors.
     */
    [[nodiscard]] const std::vector<std::uint32_t> &partitionDescriptors() const;

    /**
     * Report the sector of the logical volume integrity descriptor.
     *
     * @return The sector, or nothing when the volume has no integrity descriptor.
     */
    [[nodiscard]] std::optional<std::uint32_t> integrityDescriptor() const;

    /**
     * Give a file entry a new single extent and date.
     *
     * The information length, the logical blocks recorded, the extent of the allocation descriptor,
     * the modification time, and the attribute time change.
     *
     * @param entry Sector of a file entry or an extended file entry.
     * @param size Length of the data in bytes.
     * @param block Logical block of the data within the partition.
     * @param modified Date of the data, in UTC.
     */
    static void
    setFile(Volume::Sector &entry, std::uint32_t size, std::uint32_t block, Seconds modified);

    /**
     * Raise the modification time of a file entry. A later time stays unchanged.
     *
     * @param entry Sector of a file entry or an extended file entry.
     * @param time Earliest modification time, in UTC.
     */
    static void raiseModified(Volume::Sector &entry, Seconds time);

    /**
     * Raise the recording time of a logical volume integrity descriptor and set its partition size.
     * A later recording time stays unchanged.
     *
     * @param descriptor Sector of the integrity descriptor.
     * @param time Earliest recording time, in UTC.
     * @param partitionLength Length of the partition in blocks.
     */
    static void
    raiseIntegrity(Volume::Sector &descriptor, Seconds time, std::uint32_t partitionLength);

    /**
     * Read the length of a partition descriptor.
     *
     * @param descriptor Sector of the partition descriptor.
     * @return Length of the partition in blocks.
     */
    static std::uint32_t partitionLength(const Volume::Sector &descriptor);

    /**
     * Set the length of a partition descriptor.
     *
     * @param descriptor Sector of the partition descriptor.
     * @param length Length of the partition in blocks.
     */
    static void setPartitionLength(Volume::Sector &descriptor, std::uint32_t length);

    /**
     * Move a descriptor to another sector by rewriting its tag location.
     *
     * @param descriptor Sector of the descriptor.
     * @param location New tag location.
     */
    static void relocate(Volume::Sector &descriptor, std::uint32_t location);

private:
    UdfTree() = default;

    std::expected<void, Error> walk(Volume &volume, std::uint32_t block);
    std::expected<std::vector<std::uint8_t>, Error> directoryData(Volume &volume,
                                                                  const Volume::Sector &entry);

    std::uint32_t partitionStart_ = 0;
    std::vector<std::uint32_t> partitionDescriptors_;
    std::optional<std::uint32_t> integrityDescriptor_;
    std::map<std::uint32_t, File> files_;
};

} // namespace Tools::BuildImage

#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "isodate.h"
#include "volume.h"

namespace Tools::BuildImage {

using Common::Seconds;
using Common::Volume;

/** A file or directory of the UDF file system UdfWriter records. */
struct UdfNode {
    std::string name;       /*!< Name with its original case. The root has none. */
    bool directory = false; /*!< Whether the node is a directory. */
    std::size_t parent = 0; /*!< Index of the containing directory. The root is its own. */
    std::vector<std::size_t> children; /*!< Indices of a directory's entries, in recording order. */
    std::uint32_t entryLba = 0;        /*!< Sector of the file entry. */
    std::uint32_t lba = 0;  /*!< First sector of the data, or of a directory's records. */
    std::uint64_t size = 0; /*!< Bytes of the data, or of a directory's records. */
    Seconds modified;       /*!< Modification time, in UTC. */
};

/**
 * Writer of the UDF 1.02 half of a UDF bridge DVD image, laid out as the discs Sony's DVD-ROM
 * generator mastered.
 *
 * The volume recognition sequence follows the ISO9660 descriptors at sector 18, the main and
 * reserve volume descriptor sequences start at sectors 32 and 48, the integrity sequence at sector
 * 64, and the anchor at sector 256 and in the last sector. The partition opens with the file set
 * descriptor and a terminator, and the file data is the data the ISO9660 records give.
 */
class UdfWriter {
public:
    /** Sectors of the partition before the first directory: the file set descriptor and a
     * terminating descriptor. */
    static constexpr std::uint32_t kPartitionHeaderSectors = 2;

    /** First sector after the anchor, where the ISO9660 path tables may begin. */
    static constexpr std::uint32_t kFirstFreeSector = 257;

    /**
     * Compute the bytes of a directory's file identifier descriptors.
     *
     * @param nodes Every node, the root first.
     * @param directory Index of the directory.
     * @return The bytes.
     */
    static std::uint64_t directorySize(const std::vector<UdfNode> &nodes, std::size_t directory);

    /**
     * Write the UDF descriptors and directories.
     *
     * @param nodes Every node, the root first, with its sectors placed.
     * @param partitionStart First sector of the partition.
     * @param volumeSectors Sectors of the volume. The last one is the anchor copy, and the
     * partition ends before it.
     * @param recorded Recording time of the volume, in UTC.
     * @param sectors Destination of the written sectors.
     */
    static void write(const std::vector<UdfNode> &nodes,
                      std::uint32_t partitionStart,
                      std::uint32_t volumeSectors,
                      Seconds recorded,
                      std::map<std::uint32_t, Volume::Sector> &sectors);
};

} // namespace Tools::BuildImage

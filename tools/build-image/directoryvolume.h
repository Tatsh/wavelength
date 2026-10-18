#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "imagebuilder.h"
#include "isodate.h"
#include "volume.h"

namespace Tools::BuildImage {

using Common::Error;
using Common::LocatedFile;
using Common::Seconds;
using Common::Volume;

/**
 * A UDF bridge volume, ISO9660 and UDF 1.02, built from the files of a disc root directory.
 *
 * The layout is the layout of the discs Sony's DVD-ROM generator mastered. The ISO9660 descriptors
 * and the UDF volume structures take their usual sectors, the path tables follow the UDF anchor,
 * and the ISO9660 directories follow in path table order. The UDF partition comes next, with the
 * UDF directories, a file entry for each directory and file, and then the file data in the same
 * order, which both file systems record. ISO9660 names are upper case, and UDF names keep the case
 * of the source files. Metadata sectors are built in memory, and file sectors are read from the
 * source files or the replacement payloads on demand. Each record has the modification time of its
 * file, and a directory has the newest time of itself and every entry beneath it.
 */
class DirectoryVolume final : public Volume {
public:
    /**
     * Build the volume layout.
     *
     * @param root Disc root directory with `SYSTEM.CNF`.
     * @param systemArea The 16 system area sectors (32768 bytes) with the boot logo. The sectors
     * are zero when omitted.
     * @param replacements Files laid out with a payload's contents, size, and modification time
     * in place of the disc root's file of the same name.
     * @return The volume, or an error when the directory is not a disc root, a name is not valid,
     * a file cannot be examined, the system area has the wrong size, or a replacement does not
     * match exactly one file.
     */
    static std::expected<std::unique_ptr<DirectoryVolume>, Error>
    create(const std::filesystem::path &root,
           std::optional<std::vector<std::uint8_t>> systemArea,
           std::vector<Replacement> replacements);

    /**
     * Read the name of the executable a disc root boots, without building the volume.
     *
     * @param root Disc root directory with `SYSTEM.CNF`.
     * @return File name from the `BOOT2` line of `SYSTEM.CNF`, without the version suffix, or an
     * error when the directory lacks `SYSTEM.CNF` or `SYSTEM.CNF` does not boot a known release.
     */
    static std::expected<std::string, Error> identify(const std::filesystem::path &root);

    /** @copydoc Volume::find */
    std::expected<LocatedFile, Error> find(const std::string &name) override;
    /** @copydoc Volume::readSector */
    std::expected<void, Error> readSector(std::uint32_t lba,
                                          std::span<std::uint8_t, kSectorData> sector) override;
    /** @copydoc Volume::volumeSectors */
    [[nodiscard]] std::uint32_t volumeSectors() const override;

private:
    struct Entry {
        std::string isoName;
        std::uint32_t lba = 0;
        Seconds mtime;
        std::filesystem::path path;
        std::uint32_t size = 0;
        const std::vector<std::uint8_t> *payload = nullptr;
    };

    struct ListingEntry {
        bool isDirectory = false;
        std::string isoName;
        std::filesystem::path path;
        std::size_t index = 0;
    };

    DirectoryVolume() = default;

    std::expected<void, Error> build(const std::filesystem::path &root);
    std::expected<void, Error> applyReplacements();
    static std::expected<std::string, Error> isoName(const std::filesystem::path &entry);
    static std::uint32_t extentSize(const std::vector<ListingEntry> &listing);
    static std::vector<std::uint8_t>
    record(std::string_view identifier, const Entry &entry, bool isDirectory);
    void writeDescriptors(std::uint32_t pathTable, std::uint32_t tableSectors, Seconds newest);
    void writePathTables(std::uint32_t tableSectors);
    void writeDirectory(std::size_t directory);

    std::vector<std::uint8_t> systemArea_;
    std::vector<Replacement> replacements_;
    std::vector<Entry> directories_;
    std::vector<std::size_t> parents_;
    std::vector<std::vector<ListingEntry>> listings_;
    std::vector<Entry> files_;
    std::map<std::string, std::size_t> parentOf_;
    std::vector<std::uint32_t> fileStarts_;
    std::map<std::uint32_t, Sector> metadata_;
    std::map<std::filesystem::path, std::ifstream> handles_;
    std::uint32_t volumeSectors_ = 0;
};

} // namespace Tools::BuildImage

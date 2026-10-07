#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "volume.h"

namespace Tools::BuildImage {

/** Random access to the sectors of an ISO disc image. */
class SourceImage final : public Volume {
public:
    /**
     * Open an ISO image and verify its primary volume descriptor.
     *
     * @param path ISO image path.
     * @return The open image, or an error when the image cannot be opened, the path does not have
     * the `.iso` suffix, or sector sixteen lacks the descriptor.
     */
    static std::expected<std::unique_ptr<SourceImage>, Error>
    open(const std::filesystem::path &path);

    /** @copydoc Volume::find */
    std::expected<LocatedFile, Error> find(const std::string &name) override;
    /** @copydoc Volume::readSector */
    std::expected<void, Error> readSector(std::uint32_t lba,
                                          std::span<std::uint8_t, kSectorData> sector) override;
    /** @copydoc Volume::readSectors */
    std::expected<std::size_t, Error> readSectors(std::uint32_t lba,
                                                  std::span<std::uint8_t> sectors) override;
    /** @copydoc Volume::volumeSectors */
    [[nodiscard]] std::uint32_t volumeSectors() const override;

private:
    struct Record {
        bool isDirectory = false;
        std::uint32_t lba = 0;
        bool multiExtent = false;
        std::string name;
        std::uint32_t size = 0;
    };

    explicit SourceImage(std::ifstream handle);

    std::expected<std::vector<Record>, Error> records(std::uint32_t lba, std::uint32_t size);

    std::ifstream handle_;
    std::uint32_t volumeSectors_ = 0;
};

} // namespace Tools::BuildImage

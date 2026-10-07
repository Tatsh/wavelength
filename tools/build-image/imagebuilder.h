#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "volume.h"

namespace Tools::BuildImage {

/** A file of the original disc and the payload that replaces it. */
struct Replacement {
    std::string target;                /*!< File name without the version suffix. */
    std::vector<std::uint8_t> payload; /*!< Replacement contents. */
};

/**
 * Writer of an ISO image of a volume with replaced files.
 *
 * A payload fitting its original extent overwrites the extent with zero padding, and the metadata
 * is unchanged. A larger payload moves to sectors appended after the volume, and the ISO9660
 * directory record and volume size move with the payload. The UDF structures of a DVD image are
 * copied unchanged, so a relocated file keeps its old extent in the UDF tree.
 */
class ImageBuilder {
public:
    /**
     * Place each payload and collect the sectors that change.
     *
     * @param source Open original image or disc root. It must outlive the builder.
     * @param replacements Payloads in placement order.
     * @return The planned image, or an error when a target is missing or its directory record
     * cannot be patched.
     */
    static std::expected<ImageBuilder, Error> plan(Volume &source,
                                                   std::vector<Replacement> replacements);

    /**
     * Write the ISO image and check the replaced extents.
     *
     * @param output Destination ISO image path.
     * @return Sectors in the output image, or an error when the source ends early, the file cannot
     * be written, or a replaced extent does not open with its payload.
     */
    std::expected<std::uint32_t, Error> write(const std::filesystem::path &output);

private:
    ImageBuilder(Volume &source, std::vector<Replacement> replacements);

    std::expected<void, Error> placePayloads();
    static bool patchRecord(Volume::Sector &sector,
                            std::uint32_t oldLba,
                            const std::string &name,
                            std::uint32_t newLba,
                            std::uint32_t newSize);
    [[nodiscard]] std::expected<void, Error> verify(const std::filesystem::path &image) const;

    Volume *source_;
    std::vector<Replacement> replacements_;
    std::map<std::uint32_t, Volume::Sector> overlays_;
    std::uint32_t volumeSectors_ = 0;
    std::vector<std::pair<std::string, std::uint32_t>> written_;
};

} // namespace Tools::BuildImage

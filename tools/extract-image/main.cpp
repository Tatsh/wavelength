#include <cstdint>
#include <cstdlib>
#include <exception>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "fileio.h"
#include "imageerror.h"
#include "logging.h"
#include "sourceimage.h"

namespace {

constexpr int kUsageErrorStatus = 2;

} // namespace

int main(int argc, char *argv[]) {
    using namespace Tools::Common;
    argparse::ArgumentParser parser("extract-image", "", argparse::default_arguments::help);
    parser.add_description("Copy every file of a PlayStation 2 ISO image into a directory.");
    parser.add_argument("input_image").help("ISO image to read.");
    parser.add_argument("output_directory").help("Directory to write. It is created when missing.");
    parser.add_argument("--system-area")
        .metavar("FILE")
        .help("Also write the image's first 16 sectors, the system area with the boot logo, to "
              "FILE.");
    parser.add_argument("--debug").help("Enable debug logging.").flag();
    // argparse reports usage errors only by throwing.
    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << parser.usage() << "\nextract-image: error: " << e.what() << '\n';
        return kUsageErrorStatus;
    }
    setupLogging(parser.get<bool>("--debug"));
    const std::filesystem::path output = parser.get<std::string>("output_directory");
    auto image = SourceImage::open(parser.get<std::string>("input_image"));
    if (!image) {
        spdlog::error("Extraction failed. {}", image.error().message);
        return EXIT_FAILURE;
    }
    const auto extracted = (*image)->extract(output);
    if (!extracted) {
        spdlog::error("Extraction failed. {}", extracted.error().message);
        return EXIT_FAILURE;
    }
    if (const auto systemAreaPath = parser.present("--system-area")) {
        std::vector<std::uint8_t> systemArea(std::size_t{Volume::kPvdSector} * Volume::kSectorData);
        const auto written =
            (*image)->readSectors(0, systemArea).and_then([&](std::size_t sectors) {
                if (sectors != Volume::kPvdSector) {
                    return std::expected<void, Error>(
                        discImageError("The image ends inside its system area."));
                }
                return writeFile(*systemAreaPath, systemArea);
            });
        if (!written) {
            spdlog::error("Writing the system area failed. {}", written.error().message);
            return EXIT_FAILURE;
        }
    }
    std::cout << std::format("Extracted {} files to {}.\n", *extracted, output.string());
    return EXIT_SUCCESS;
}

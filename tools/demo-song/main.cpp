#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "demosong.h"
#include "logging.h"

namespace {

constexpr int kUsageErrorStatus = 2;

} // namespace

int main(int argc, char *argv[]) {
    using namespace Tools::Common;
    using namespace Tools::DemoSong;
    argparse::ArgumentParser parser("demo-song", "", argparse::default_arguments::help);
    parser.add_description(
        "Add the P.O.D. demo's exclusive song, \"Space\", to the extracted files "
        "of an Amplitude disc and of its archive, as Amplitude-Space-Edition "
        "does.");
    parser.add_argument("ark_root").help("The extracted files of the retail gen/main.ark.");
    parser.add_argument("disc_root").help("The extracted files of the retail disc.");
    parser.add_argument("--demo-image")
        .metavar("DEMO_IMAGE")
        .help("The P.O.D. demo ISO image, or its disc root.")
        .required();
    parser.add_argument("--space-edition")
        .metavar("SPACE_EDITION")
        .help("The Amplitude-Space-Edition checkout, for its streams and song entry.")
        .required();
    parser.add_argument("--pal").help("The disc is the European release.").flag();
    parser.add_argument("--debug").help("Enable debug logging.").flag();
    // argparse reports usage errors only by throwing.
    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << parser.usage() << "\ndemo-song: error: " << e.what() << '\n';
        return kUsageErrorStatus;
    }
    setupLogging(parser.get<bool>("--debug"));
    const Inputs inputs{parser.get<std::string>("ark_root"),
                        parser.get<std::string>("disc_root"),
                        parser.get<std::string>("--demo-image"),
                        parser.get<std::string>("--space-edition"),
                        parser.get<bool>("--pal")};
    if (const auto added = Tools::DemoSong::DemoSong::add(inputs); !added) {
        spdlog::error("Adding the song failed. {}", added.error().message);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

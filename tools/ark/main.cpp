#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "arkreader.h"
#include "arkwriter.h"
#include "fileio.h"
#include "imageerror.h"
#include "logging.h"

namespace Tools::Ark {

using Common::Error;
using Common::invalidInput;
using Common::readLines;
using Common::setupLogging;
using Common::writeLines;

namespace {

namespace fs = std::filesystem;

constexpr int kUsageErrorStatus = 2;

std::expected<void, Error> list(const fs::path &archive) {
    return ArkReader::open(archive).transform([](const ArkReader &reader) {
        for (const auto &entry : reader.entries()) {
            std::cout << std::format("{:#010x} {:>10} {:>10} {}\n",
                                     entry.offset,
                                     entry.size,
                                     entry.uncompressedSize,
                                     entry.path);
        }
    });
}

// The order file lists the paths by data offset, so create() can reproduce the layout.
std::expected<void, Error>
extract(const fs::path &archive, const fs::path &root, const std::optional<std::string> &order) {
    auto reader = ArkReader::open(archive);
    if (!reader) {
        return std::unexpected(reader.error());
    }
    if (auto extracted = reader->extract(root); !extracted) {
        return extracted;
    }
    if (!order) {
        return {};
    }
    auto entries = reader->entries();
    std::ranges::stable_sort(entries, {}, &ArkEntry::offset);
    std::vector<std::string> paths;
    for (const auto &entry : entries) {
        paths.push_back(entry.path);
    }
    return writeLines(*order, paths);
}

std::expected<std::size_t, Error>
create(const fs::path &root, const fs::path &archive, const std::optional<std::string> &orderFile) {
    std::vector<std::string> order;
    if (orderFile) {
        auto lines = readLines(*orderFile);
        if (!lines) {
            return std::unexpected(lines.error());
        }
        order = std::move(*lines);
    }
    const auto written = ArkWriter::write(root, order, archive);
    if (!written) {
        return written;
    }
    // Every file must resolve through the hash table the way the game looks it up.
    auto reader = ArkReader::open(archive);
    if (!reader) {
        return std::unexpected(reader.error());
    }
    for (const auto &entry : reader->entries()) {
        const auto found = reader->locate(entry.path);
        if (!found || found->offset != entry.offset || found->size != entry.size) {
            return invalidInput(std::format(
                "{} does not resolve to its record in {}.", entry.path, archive.string()));
        }
    }
    return written;
}

} // namespace

} // namespace Tools::Ark

int main(int argc, char *argv[]) {
    using namespace Tools::Common;
    using namespace Tools::Ark;
    argparse::ArgumentParser parser("ark", "", argparse::default_arguments::help);
    parser.add_description("List, extract, and create the version 2 archives of Amplitude.");
    parser.add_argument("--debug").help("Enable debug logging.").flag();

    argparse::ArgumentParser listCommand("list", "", argparse::default_arguments::help);
    listCommand.add_description("Print the offset, size, inflated size, and path of each file.");
    listCommand.add_argument("archive").help("The archive.");

    argparse::ArgumentParser extractCommand("extract", "", argparse::default_arguments::help);
    extractCommand.add_description("Copy every file of an archive into a directory.");
    extractCommand.add_argument("archive").help("The archive.");
    extractCommand.add_argument("directory").help("The directory to write.");
    extractCommand.add_argument("--order").metavar("ORDER").help(
        "Also write the paths in data order to ORDER, for create --order.");

    argparse::ArgumentParser createCommand("create", "", argparse::default_arguments::help);
    createCommand.add_description("Write an archive of every file below a directory.");
    createCommand.add_argument("directory").help("The directory with the files.");
    createCommand.add_argument("archive").help("The archive to write.");
    createCommand.add_argument("--order").metavar("ORDER").help(
        "File listing the paths to place first, one per line, in order.");

    parser.add_subparser(listCommand);
    parser.add_subparser(extractCommand);
    parser.add_subparser(createCommand);
    // argparse reports usage errors only by throwing.
    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << parser.usage() << "\nark: error: " << e.what() << '\n';
        return kUsageErrorStatus;
    }
    setupLogging(parser.get<bool>("--debug"));
    std::expected<void, Error> result;
    if (parser.is_subcommand_used(listCommand)) {
        result = list(listCommand.get<std::string>("archive"));
    } else if (parser.is_subcommand_used(extractCommand)) {
        result = extract(extractCommand.get<std::string>("archive"),
                         extractCommand.get<std::string>("directory"),
                         extractCommand.present<std::string>("--order"));
    } else if (parser.is_subcommand_used(createCommand)) {
        result = create(createCommand.get<std::string>("directory"),
                        createCommand.get<std::string>("archive"),
                        createCommand.present<std::string>("--order"))
                     .transform([](std::size_t files) { spdlog::info("Wrote {} files.", files); });
    } else {
        std::cerr << parser;
        return kUsageErrorStatus;
    }
    if (!result) {
        spdlog::error("{}", result.error().message);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

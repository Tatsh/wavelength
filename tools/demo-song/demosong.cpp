#include "demosong.h"

#include <algorithm>
#include <array>
#include <format>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "arkreader.h"
#include "datafile.h"
#include "datatext.h"
#include "fileio.h"
#include "imageerror.h"
#include "sourceimage.h"
#include "volume.h"

namespace Tools::DemoSong {

using Common::Error;
using Common::invalidInput;
using Common::ioError;
using Common::readFile;
using Common::SourceImage;
using Common::Volume;
using Common::writeFile;

namespace {

namespace fs = std::filesystem;
using Ark::ArkReader;
using Ark::DataFile;
using Ark::DataNode;

constexpr std::string_view kSong = "POD_DEMO";
constexpr std::string_view kStringPrefix = "POD_DEMO_";
constexpr std::string_view kSongDirectory = "songs/pod_demo/";
constexpr std::string_view kDemoArchiveName = "MAIN.ARK";
constexpr std::string_view kDemoArchivePath = "gen/main.ark";
constexpr std::string_view kNtscConfig = "gen/freq2_config.txt.bin";
constexpr std::string_view kPalConfig = "gen/cfg_freq2euro.txt.bin";
constexpr std::string_view kNtscStrings = "gen/english_songs.txt.bin";
constexpr std::array<std::string_view, 5> kPalStrings{"gen/english_songs.txt.bin",
                                                      "gen/french_songs.txt.bin",
                                                      "gen/german_songs.txt.bin",
                                                      "gen/italian_songs.txt.bin",
                                                      "gen/spanish_songs.txt.bin"};
constexpr std::string_view kDatabaseKey = "db";
constexpr std::string_view kSongsKey = "songs";

// The parts of the Amplitude-Space-Edition checkout.
constexpr std::string_view kSpaceEditionArk = "ark";
constexpr std::string_view kSpaceEditionDisc = "disc";
constexpr std::string_view kSpaceEditionGen = "gen";
constexpr std::string_view kSpaceEditionConfig = "ark/gen/freq2_config.dta";
constexpr std::string_view kSpaceEditionStrings = "ark/gen/english_songs.dta";

bool sameName(const fs::path &left, const fs::path &right) {
    return Volume::upperAscii(left.string()) == Volume::upperAscii(right.string());
}

// Resolve each component to an existing entry of any case. The components past the last match
// keep the case of the relative path.
fs::path findCaseless(const fs::path &root, const fs::path &relative) {
    auto resolved = root;
    auto it = relative.begin();
    for (; it != relative.end(); ++it) {
        std::error_code error;
        fs::path match;
        for (const auto &entry : fs::directory_iterator(resolved, error)) {
            if (sameName(entry.path().filename(), *it)) {
                match = entry.path();
                break;
            }
        }
        if (match.empty()) {
            break;
        }
        resolved = match;
    }
    for (; it != relative.end(); ++it) {
        resolved /= *it;
    }
    return resolved;
}

std::expected<void, Error> writeCreating(const fs::path &path, std::span<const std::uint8_t> data) {
    std::error_code error;
    fs::create_directories(path.parent_path(), error);
    if (error) {
        return ioError(std::format("Cannot create {}.", path.parent_path().string()));
    }
    return writeFile(path, data);
}

std::expected<ArkReader, Error> openDemoArchive(const fs::path &image) {
    std::error_code error;
    if (fs::is_directory(image, error)) {
        return ArkReader::open(findCaseless(image, kDemoArchivePath));
    }
    auto source = SourceImage::open(image);
    if (!source) {
        return std::unexpected(source.error());
    }
    const auto found = (*source)->find(std::string(kDemoArchiveName));
    if (!found) {
        return std::unexpected(found.error());
    }
    return ArkReader::open(image, std::uint64_t{found->lba} * Volume::kSectorData);
}

std::expected<std::size_t, Error> copySongFiles(ArkReader &demo, const fs::path &arkRoot) {
    std::size_t copied = 0;
    for (const auto &entry : demo.entries()) {
        if (!entry.path.starts_with(kSongDirectory)) {
            continue;
        }
        const auto data = demo.read(entry);
        if (!data) {
            return std::unexpected(data.error());
        }
        if (auto written = writeCreating(arkRoot / entry.path, *data); !written) {
            return std::unexpected(written.error());
        }
        ++copied;
    }
    if (copied == 0) {
        return invalidInput(std::format("The demo archive has no {} files. Is it the P.O.D. demo?",
                                        kSongDirectory));
    }
    return copied;
}

// Copy every file below from over the file of the same path below to, matching names in any case.
// Paths below skip are left out.
std::expected<std::size_t, Error>
copyOverlay(const fs::path &from, const fs::path &to, const fs::path &skip) {
    std::size_t copied = 0;
    std::error_code error;
    for (fs::recursive_directory_iterator it(from, error), end; !error && it != end;
         it.increment(error)) {
        const auto relative = it->path().lexically_relative(from);
        if (!it->is_regular_file(error) || (!skip.empty() && *relative.begin() == skip)) {
            continue;
        }
        const auto target = findCaseless(to, relative);
        fs::create_directories(target.parent_path(), error);
        if (error ||
            !fs::copy_file(it->path(), target, fs::copy_options::overwrite_existing, error)) {
            return ioError(
                std::format("Cannot copy {} to {}.", it->path().string(), target.string()));
        }
        ++copied;
    }
    if (error) {
        return ioError(std::format("Cannot list {}.", from.string()));
    }
    return copied;
}

std::expected<DataFile, Error> readText(const fs::path &path) {
    return readFile(path).and_then(
        [&path](const std::vector<std::uint8_t> &data) -> std::expected<DataFile, Error> {
            auto parsed = Ark::DataText::parse(
                std::string_view(reinterpret_cast<const char *>(data.data()), data.size()));
            if (!parsed) {
                return invalidInput(std::format("{}: {}", path.string(), parsed.error().message));
            }
            return parsed;
        });
}

// The writer must reproduce an untouched file byte for byte, or the splice would change more than
// the song.
std::expected<DataFile, Error> readCompiled(const fs::path &path) {
    const auto data = readFile(path);
    if (!data) {
        return std::unexpected(data.error());
    }
    auto parsed = DataFile::parse(*data);
    if (!parsed) {
        return invalidInput(std::format("{}: {}", path.string(), parsed.error().message));
    }
    if (parsed->serialize() != *data) {
        return invalidInput(
            std::format("{} does not round-trip through the writer.", path.string()));
    }
    return parsed;
}

DataNode *songArenas(DataFile &file) {
    auto *database = file.root.findArray(kDatabaseKey);
    return database == nullptr ? nullptr : database->findArray(kSongsKey);
}

bool isString(const DataNode &node) {
    return node.type == DataNode::Type::Array && !node.children.empty() &&
           node.children.front().type == DataNode::Type::Symbol &&
           node.children.front().symbol.starts_with(kStringPrefix);
}

// Insert nodes after the child that opens with after, or at the end when there is none.
void insertAfter(DataNode &parent, std::string_view after, std::vector<DataNode> nodes) {
    const auto found = parent.indexOf(after);
    const auto at = found < 0 ? parent.children.size() : static_cast<std::size_t>(found) + 1;
    parent.children.insert(parent.children.begin() + static_cast<std::ptrdiff_t>(at),
                           std::make_move_iterator(nodes.begin()),
                           std::make_move_iterator(nodes.end()));
}

std::string predecessor(const DataNode &parent, std::size_t index) {
    if (index == 0 || parent.children[index - 1].type != DataNode::Type::Array ||
        parent.children[index - 1].children.empty()) {
        return {};
    }
    return parent.children[index - 1].children.front().symbol;
}

std::expected<void, Error> patchConfig(const fs::path &path, const DataFile &spaceEdition) {
    auto target = readCompiled(path);
    if (!target) {
        return std::unexpected(target.error());
    }
    auto source = spaceEdition;
    auto *sourceArenas = songArenas(source);
    auto *targetArenas = songArenas(*target);
    if (sourceArenas == nullptr || targetArenas == nullptr) {
        return invalidInput(std::format("A configuration lacks ({} ({}.", kDatabaseKey, kSongsKey));
    }
    for (const auto &arena : sourceArenas->children) {
        const auto index = arena.indexOf(kSong);
        if (index < 0) {
            continue;
        }
        const auto &arenaName = arena.children.front().symbol;
        auto *targetArena = targetArenas->findArray(arenaName);
        if (targetArena == nullptr) {
            return invalidInput(std::format("{} lacks the arena {}.", path.string(), arenaName));
        }
        auto song = arena.children[static_cast<std::size_t>(index)];
        song.remapFiles([&](std::int32_t file) {
            return file >= 0 && static_cast<std::size_t>(file) < source.files.size() ?
                       target->fileIndex(source.files[static_cast<std::size_t>(file)]) :
                       file;
        });
        std::erase_if(targetArena->children,
                      [](const DataNode &node) { return node.isKeyed(kSong); });
        insertAfter(*targetArena, predecessor(arena, static_cast<std::size_t>(index)), {song});
        spdlog::info("Added {} to the arena {} of {}.", kSong, arenaName, path.string());
        return writeFile(path, target->serialize());
    }
    return invalidInput(std::format("The Amplitude-Space-Edition configuration lacks {}.", kSong));
}

std::expected<void, Error> patchStrings(const fs::path &path, const DataFile &spaceEdition) {
    auto target = readCompiled(path);
    if (!target) {
        return std::unexpected(target.error());
    }
    const auto &children = spaceEdition.root.children;
    const auto first = std::ranges::find_if(children, isString);
    if (first == children.end()) {
        return invalidInput(
            std::format("The Amplitude-Space-Edition strings lack {}*.", kStringPrefix));
    }
    std::vector<DataNode> strings;
    std::ranges::copy_if(children, std::back_inserter(strings), isString);
    std::erase_if(target->root.children, isString);
    const auto after =
        predecessor(spaceEdition.root, static_cast<std::size_t>(first - children.begin()));
    // A string table has one source file, and the strings take the file of their neighbours.
    const auto *neighbour = target->root.findArray(after);
    const auto file = neighbour != nullptr ? neighbour->file : 0;
    for (auto &string : strings) {
        string.remapFiles([file](std::int32_t) { return file; });
    }
    const auto count = strings.size();
    insertAfter(target->root, after, std::move(strings));
    spdlog::info("Added {} strings to {}.", count, path.string());
    return writeFile(path, target->serialize());
}

} // namespace

std::expected<void, Error> DemoSong::add(const Inputs &inputs) {
    auto demo = openDemoArchive(inputs.demoImage);
    if (!demo) {
        return std::unexpected(demo.error());
    }
    const auto songFiles = copySongFiles(*demo, inputs.arkRoot);
    if (!songFiles) {
        return std::unexpected(songFiles.error());
    }
    spdlog::info("Copied {} song files from the demo.", *songFiles);
    const auto config = readText(inputs.spaceEdition / kSpaceEditionConfig);
    if (!config) {
        return std::unexpected(config.error());
    }
    const auto strings = readText(inputs.spaceEdition / kSpaceEditionStrings);
    if (!strings) {
        return std::unexpected(strings.error());
    }
    for (const auto &[from, to, skip] :
         {std::tuple{kSpaceEditionArk, inputs.arkRoot, kSpaceEditionGen},
          std::tuple{kSpaceEditionDisc, inputs.discRoot, std::string_view()}}) {
        const auto copied = copyOverlay(inputs.spaceEdition / from, to, skip);
        if (!copied) {
            return std::unexpected(copied.error());
        }
        if (*copied == 0) {
            return invalidInput(std::format("Amplitude-Space-Edition has no files in {}.", from));
        }
        spdlog::info("Copied {} Amplitude-Space-Edition files into {}.", *copied, to.string());
    }
    if (auto patched =
            patchConfig(inputs.arkRoot / (inputs.pal ? kPalConfig : kNtscConfig), *config);
        !patched) {
        return patched;
    }
    const auto tables = inputs.pal ? std::span<const std::string_view>(kPalStrings) :
                                     std::span<const std::string_view>(&kNtscStrings, 1);
    for (const auto table : tables) {
        if (auto patched = patchStrings(inputs.arkRoot / table, *strings); !patched) {
            return patched;
        }
    }
    return {};
}

} // namespace Tools::DemoSong

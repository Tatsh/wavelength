#include "imagebuilder.h"

#include <algorithm>
#include <array>
#include <format>
#include <fstream>
#include <span>
#include <string_view>

#include <spdlog/spdlog.h>

#include "byteorder.h"
#include "imageerror.h"

namespace Tools::BuildImage {

namespace {

constexpr std::size_t kBatchSectors = 1024;
constexpr std::string_view kElfMagic = "\x7f"
                                       "ELF";
constexpr std::size_t kVerifyLength = 4;
constexpr std::size_t kPvdVolumeSectors = 80;

// The offsets below are within a directory record.
constexpr std::size_t kRecordLba = 2;
constexpr std::size_t kRecordSize = 10;
constexpr std::size_t kRecordNameLength = 32;
constexpr std::size_t kRecordName = 33;

} // namespace

ImageBuilder::ImageBuilder(Volume &source, std::vector<Replacement> replacements)
    : source_(&source), replacements_(std::move(replacements)) {
}

std::expected<ImageBuilder, Error> ImageBuilder::plan(Volume &source,
                                                      std::vector<Replacement> replacements) {
    ImageBuilder builder(source, std::move(replacements));
    if (auto placed = builder.placePayloads(); !placed) {
        return std::unexpected(std::move(placed.error()));
    }
    return builder;
}

std::expected<void, Error> ImageBuilder::placePayloads() {
    constexpr auto kSectorData = Volume::kSectorData;
    const auto sourceSectors = source_->volumeSectors();
    auto appendLba = sourceSectors;
    for (const auto &[target, payload] : replacements_) {
        if (!std::string_view(reinterpret_cast<const char *>(payload.data()), payload.size())
                 .starts_with(kElfMagic)) {
            spdlog::warn("Payload for `{}` lacks an ELF header.", target);
        }
        const auto found = source_->find(target);
        if (!found) {
            return std::unexpected(found.error());
        }
        if (payload.size() <= found->size) {
            for (std::uint32_t index = 0; index < Volume::sectorsFor(found->size); ++index) {
                const auto start = std::size_t{index} * kSectorData;
                const auto end = std::min(start + kSectorData, std::size_t{found->size});
                Volume::Sector chunk{};
                if (end - start < kSectorData) {
                    if (auto read = source_->readSector(found->lba + index, chunk); !read) {
                        return read;
                    }
                }
                for (auto i = start; i < end; ++i) {
                    chunk[i - start] = i < payload.size() ? payload[i] : 0;
                }
                overlays_[found->lba + index] = chunk;
            }
            written_.emplace_back(target, found->lba);
            spdlog::info("Replaced {} in place ({} bytes).", target, payload.size());
            continue;
        }
        const auto sectors = Volume::sectorsFor(payload.size());
        for (std::uint32_t index = 0; index < sectors; ++index) {
            Volume::Sector chunk{};
            const auto slice = std::span(payload).subspan(std::size_t{index} * kSectorData);
            std::ranges::copy(slice.first(std::min(slice.size(), kSectorData)), chunk.begin());
            overlays_[appendLba + index] = chunk;
        }
        auto patched = false;
        for (std::uint32_t index = 0; !patched && index < Volume::sectorsFor(found->parentSize);
             ++index) {
            const auto parentLba = found->parentLba + index;
            Volume::Sector parent;
            if (const auto overlay = overlays_.find(parentLba); overlay != overlays_.end()) {
                parent = overlay->second;
            } else if (auto read = source_->readSector(parentLba, parent); !read) {
                return read;
            }
            if (patchRecord(parent,
                            found->lba,
                            target,
                            appendLba,
                            static_cast<std::uint32_t>(payload.size()))) {
                overlays_[parentLba] = parent;
                patched = true;
            }
        }
        if (!patched) {
            return discImageError(
                std::format("The directory record of {} is missing from its directory.", target));
        }
        written_.emplace_back(target, appendLba);
        spdlog::info("Relocated {} to sector {} ({} bytes).", target, appendLba, payload.size());
        appendLba += sectors;
    }
    if (appendLba != sourceSectors) {
        Volume::Sector descriptor;
        if (auto read = source_->readSector(Volume::kPvdSector, descriptor); !read) {
            return read;
        }
        writeLittle(descriptor, kPvdVolumeSectors, appendLba);
        writeBig(descriptor, kPvdVolumeSectors + 4, appendLba);
        overlays_[Volume::kPvdSector] = descriptor;
    }
    volumeSectors_ = appendLba;
    return {};
}

std::expected<std::uint32_t, Error> ImageBuilder::write(const std::filesystem::path &output) {
    constexpr auto kSectorData = Volume::kSectorData;
    std::ofstream file(output, std::ios::binary | std::ios::trunc);
    if (!file) {
        return ioError(std::format("Cannot write {}.", output.string()));
    }
    const auto readLimit = std::min(source_->volumeSectors(), volumeSectors_);
    std::vector<std::uint8_t> sectors(kBatchSectors * kSectorData);
    for (std::uint32_t lba = 0; lba < volumeSectors_;) {
        const auto count = std::min<std::size_t>(kBatchSectors, volumeSectors_ - lba);
        const auto readable = lba < readLimit ? std::min<std::size_t>(count, readLimit - lba) : 0;
        std::size_t available = 0;
        if (readable != 0) {
            const auto read =
                source_->readSectors(lba, std::span(sectors).first(readable * kSectorData));
            if (!read) {
                return std::unexpected(read.error());
            }
            available = *read;
        }
        for (std::size_t i = 0; i < count; ++i) {
            const auto current = static_cast<std::uint32_t>(lba + i);
            const auto sector = std::span(sectors).subspan(i * kSectorData, kSectorData);
            if (const auto found = overlays_.find(current); found != overlays_.end()) {
                std::ranges::copy(found->second, sector.begin());
            } else if (i >= available) {
                return Volume::pastEnd(current);
            }
        }
        // The streams write char.
        file.write(reinterpret_cast<const char *>(sectors.data()),
                   static_cast<std::streamsize>(count * kSectorData));
        if (!file) {
            return ioError(std::format("Cannot write {}.", output.string()));
        }
        lba += static_cast<std::uint32_t>(count);
    }
    file.close();
    if (!file) {
        return ioError(std::format("Cannot write {}.", output.string()));
    }
    return verify(output).transform([this] { return volumeSectors_; });
}

bool ImageBuilder::patchRecord(Volume::Sector &sector,
                               std::uint32_t oldLba,
                               const std::string &name,
                               std::uint32_t newLba,
                               std::uint32_t newSize) {
    std::size_t offset = 0;
    while (offset + kRecordName <= sector.size()) {
        const auto length = sector[offset];
        if (length == 0) {
            break;
        }
        const auto nameEnd =
            std::min(offset + kRecordName + sector[offset + kRecordNameLength], sector.size());
        const std::string entryName(sector.begin() +
                                        static_cast<std::ptrdiff_t>(offset + kRecordName),
                                    sector.begin() + static_cast<std::ptrdiff_t>(nameEnd));
        if (readLittle32(sector, offset + kRecordLba) == oldLba &&
            Volume::upperAscii(entryName.substr(0, entryName.find(';'))) == name) {
            writeBoth(sector, offset + kRecordLba, newLba);
            writeBoth(sector, offset + kRecordSize, newSize);
            return true;
        }
        offset += length;
    }
    return false;
}

std::expected<void, Error> ImageBuilder::verify(const std::filesystem::path &image) const {
    std::ifstream file(image, std::ios::binary);
    if (!file) {
        return ioError(std::format("Cannot read {}.", image.string()));
    }
    for (const auto &[target, lba] : written_) {
        const auto &payload =
            std::ranges::find(replacements_, target, &Replacement::target)->payload;
        std::array<char, kVerifyLength> head{};
        file.clear();
        file.seekg(static_cast<std::streamoff>(std::uint64_t{lba} * Volume::kSectorData));
        file.read(head.data(), head.size());
        const auto prefix = std::span(payload).first(std::min(payload.size(), kVerifyLength));
        if (static_cast<std::size_t>(file.gcount()) != prefix.size() ||
            !std::ranges::equal(prefix, head, [](auto left, auto right) {
                return left == static_cast<std::uint8_t>(right);
            })) {
            return discImageError(std::format("Verification failed for {}.", target));
        }
    }
    return {};
}

} // namespace Tools::BuildImage

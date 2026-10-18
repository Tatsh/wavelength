#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "error.h"

namespace Tools::Ark {

using Common::Error;

/** A file of an archive. */
struct ArkEntry {
    std::string path;         /*!< Path of the file, its directory and name joined by `/`. */
    std::uint32_t offset = 0; /*!< First byte, from the start of the archive. */
    std::uint32_t size = 0;   /*!< Size in bytes. */
    std::uint32_t uncompressedSize = 0; /*!< Size of a gzip file once inflated, or zero. */
};

/**
 * Reader of a version 2 archive, the `gen/main.ark` of Amplitude and of its P.O.D. demo.
 *
 * The archive opens with the version, the file records, the string block of the names, and the
 * name hash table, and the file data follows. A record identifies its name and its directory by
 * their hash table slots.
 */
class ArkReader {
public:
    /** The archive version the game reads. */
    static constexpr std::uint32_t kVersion = 2;

    /**
     * Open an archive and read its directory.
     *
     * @param path The archive, or a disc image that stores it.
     * @param base Offset of the archive in the file, for an archive inside a disc image.
     * @return The reader, or an error when the file cannot be read or its directory is not valid.
     */
    static std::expected<ArkReader, Error> open(const std::filesystem::path &path,
                                                std::uint64_t base = 0);

    /**
     * List the files in record order.
     *
     * @return The files.
     */
    [[nodiscard]] const std::vector<ArkEntry> &entries() const;

    /**
     * Find a file the way `Archive::GetFileInfo` does, through the name hash table.
     *
     * @param path Path of the file.
     * @return The file, or nothing when the hash table or the records lack it.
     */
    [[nodiscard]] std::optional<ArkEntry> locate(std::string_view path) const;

    /**
     * Read the bytes of a file.
     *
     * @param entry The file.
     * @return The bytes, or an error when the archive ends early.
     */
    std::expected<std::vector<std::uint8_t>, Error> read(const ArkEntry &entry);

    /**
     * Copy every file into a directory, at its path below the directory.
     *
     * @param root Destination directory. It is created when missing.
     * @return Nothing, or an error when a file cannot be read or written.
     */
    std::expected<void, Error> extract(const std::filesystem::path &root);

private:
    ArkReader() = default;

    [[nodiscard]] int slotOf(std::string_view name) const;

    std::ifstream file_;
    std::filesystem::path path_;
    std::uint64_t base_ = 0;
    std::vector<ArkEntry> entries_;
    std::vector<std::uint32_t> slots_; // The name slot and directory slot of each record.
    std::vector<char> strings_;
    std::vector<std::uint32_t> table_;
};

} // namespace Tools::Ark

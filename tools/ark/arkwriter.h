#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

#include "error.h"

namespace Tools::Ark {

using Common::Error;

/**
 * Writer of a version 2 archive from the files below a directory.
 *
 * Each file is stored at its path relative to the directory. The names of the directories and of
 * the files share one string block and one hash table, with a slot for each name the game's
 * `HashString` gives, or the next free slot. The data follows the directory without padding. A
 * gzip file records the size from its trailer as its inflated size.
 */
class ArkWriter {
public:
    /**
     * Write the archive.
     *
     * @param root Directory with the files.
     * @param order Paths to place first, in order. A path missing from the directory is skipped,
     * and the remaining files follow in byte order of their paths.
     * @param output The archive to write.
     * @return The number of files, or an error when a path is not representable, a file cannot be
     * read, the archive passes 2 GiB, or the output cannot be written.
     */
    static std::expected<std::size_t, Error> write(const std::filesystem::path &root,
                                                   const std::vector<std::string> &order,
                                                   const std::filesystem::path &output);
};

} // namespace Tools::Ark

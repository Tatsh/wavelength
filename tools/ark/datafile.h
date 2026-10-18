#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "datanode.h"
#include "error.h"

namespace Tools::Ark {

using Common::Error;

/**
 * A compiled DataArray file (`.txt.bin`, `.ui.bin`), the form `DataArray::ReadFile` loads.
 *
 * The file is a version byte, the table of source file names, and the root array. The European
 * release and the P.O.D. demo encrypt everything after a leading 32-bit seed with the lagged
 * Fibonacci generator of the game's `Rand` class.
 */
class DataFile {
public:
    /** The version of the compiled format. */
    static constexpr std::uint8_t kVersion = 2;

    /**
     * Parse a compiled file, plain or encrypted.
     *
     * @param data The file.
     * @return The parsed file, or an `ErrorCode::InvalidInput` error when neither form parses to
     * the end of the data.
     */
    static std::expected<DataFile, Error> parse(std::span<const std::uint8_t> data);

    /**
     * Write the file in compiled form, encrypted with the seed when there is one.
     *
     * @return The file.
     */
    [[nodiscard]] std::vector<std::uint8_t> serialize() const;

    /**
     * Find the index of a source file name, adding the name when missing.
     *
     * @param name The source file name.
     * @return The index in files.
     */
    std::int32_t fileIndex(const std::string &name);

    std::optional<std::uint32_t> seed; /*!< The encryption seed, when the file is encrypted. */
    std::vector<std::string> files;    /*!< The source file names that arrays refer to. */
    DataNode root;                     /*!< The root array. */
};

} // namespace Tools::Ark

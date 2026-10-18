#pragma once

#include <expected>
#include <string_view>

#include "datafile.h"
#include "error.h"

namespace Tools::Ark {

using Common::Error;

/**
 * Reader of the decompiled text form of a compiled DataArray file.
 *
 * The text opens with the source file names in braces. Each array is in parentheses, and an
 * optional `[line file]` pair after the opening parenthesis gives its source line and file index;
 * a pair before the first array gives those of the root. Elements are integers, floats, symbols in
 * single quotes, strings in double quotes, and bare words, which are symbols. A `;` starts a
 * comment that ends with the line.
 */
class DataText {
public:
    /**
     * Parse the text into a compiled file without a seed.
     *
     * @param text The text, one byte per character.
     * @return The file, or an `ErrorCode::InvalidInput` error naming the offset where parsing
     * failed.
     */
    static std::expected<DataFile, Error> parse(std::string_view text);
};

} // namespace Tools::Ark

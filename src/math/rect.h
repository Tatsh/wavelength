#pragma once

/**
 * Rectangle of four floats, the left and top edges and the size.
 *
 * The type has no RTTI, and its name is inferred.
 */
struct Rect {
    float x; /*!< Left edge. */
    float y; /*!< Top edge. */
    float w; /*!< Width. */
    float h; /*!< Height. */
};

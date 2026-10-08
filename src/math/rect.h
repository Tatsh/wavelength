#pragma once

#include "os/prnstream.h"

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

/**
 * Write the four components of a rectangle.
 *
 * @param stream The stream to write to.
 * @param rect The rectangle.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002905c0
 * @ghidraAddress PAL: 0x00299f88
 */
PrnStream &operator<<(PrnStream &stream, const Rect &rect);

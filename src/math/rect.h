#pragma once

#include "os/prnstream.h"

/**
 * Axis-aligned rectangle given by a corner and a size.
 *
 * The class is not polymorphic and emits no RTTI descriptor, so the name is inferred. The member
 * titles come from the labels of the text writer.
 */
struct Rect {
    float x; /*!< The left edge. */
    float y; /*!< The top edge. */
    float w; /*!< The width. */
    float h; /*!< The height. */
};

/**
 * Write a rectangle as `(x: y: w: h:)`.
 *
 * @param stream The stream to write to.
 * @param rect The rectangle.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002905c0
 * @ghidraAddress PAL: 0x00299f88
 */
PrnStream &operator<<(PrnStream &stream, const Rect &rect);

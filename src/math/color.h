#pragma once

#include "os/prnstream.h"

/**
 * Linear colour with four float components.
 *
 * The class is not polymorphic and emits no RTTI descriptor, so the name is inferred. The text
 * dump of a material colour and of a mesh vertex colour writes four components under the labels
 * "(r:", " g:", " b:", and " a:", and the renderer copies the four words as one quadword.
 */
struct Color {
    float r;
    float g;
    float b;
    float a;
};

/**
 * Write a colour as `(r: g: b: a:)`.
 *
 * @param stream The stream to write to.
 * @param color The colour.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00292e28
 * @ghidraAddress PAL: 0x0029c7f0
 */
PrnStream &operator<<(PrnStream &stream, const Color &color);

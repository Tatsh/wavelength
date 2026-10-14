#pragma once

#include "math/vector3.h"
#include "os/prnstream.h"

/**
 * Plane equation, padded to a PlayStation 2 quadword.
 *
 * The class is not polymorphic and emits no RTTI descriptor, so the name is inferred. Its text
 * dump writes four components under the labels "(a:", " b:", " c:", and " d:", and every vector
 * unit access loads and stores all four words at once. The first three components are the normal
 * and the fourth is the signed distance along it, which is what the plane transform at
 * `0x00550fa8` proves by rotating the first three and adjusting the fourth by the translation.
 */
struct Plane {
    float a;
    float b;
    float c;
    float d;
};

/**
 * Write a plane as `(a: b: c: d:)`.
 *
 * @param stream The stream to write to.
 * @param plane The plane.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002906e0
 * @ghidraAddress PAL: 0x0029a0a8
 */
PrnStream &operator<<(PrnStream &stream, const Plane &plane);

/**
 * Two points that fix a line or a segment.
 *
 * The routines that return one move it as two quadwords through a hidden result pointer. The name
 * is inferred.
 */
struct Segment {
    Vector3 mEnds[2]; /*!< The start, then the end. */
};

#pragma once

#include "math/plane.h"
#include "math/vector3.h"
#include "os/prnstream.h"

/**
 * The faces a segment test against a triangle discards by winding.
 *
 * The name is inferred. The text writer labels the values "CW", "CCW", and "No".
 */
enum CullMode {
    kCullClockwise = 0,        /*!< Discard faces whose normal points along the segment. */
    kCullCounterClockwise = 1, /*!< Discard faces whose normal points against the segment. */
    kCullNone = 2,             /*!< Test both windings. */
};

/**
 * Write the label of a cull mode.
 *
 * A value outside CullMode writes nothing.
 *
 * @param stream The stream to write to.
 * @param nMode The mode.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00290448
 * @ghidraAddress PAL: 0x00299e10
 */
PrnStream &operator<<(PrnStream &stream, CullMode nMode);

/**
 * Triangle stored as a corner, the two edges leaving it, and its normal.
 *
 * The class is not polymorphic and emits no RTTI descriptor, so the name and the member titles are
 * inferred from IntersectSegmentTriangle().
 */
struct Triangle {
    Vector3 mOrigin; /*!< The first corner. */
    Vector3 mEdge1;  /*!< The second corner less the first. */
    Vector3 mEdge2;  /*!< The third corner less the first. */
    Vector3 mNormal; /*!< The face normal. */
};

/**
 * Report whether a segment passes through a triangle.
 *
 * The point where the line meets the triangle's plane is projected onto the first coordinate
 * plane, of xy, xz, and yz, in which the edges are not parallel.
 *
 * @param segment The segment.
 * @param triangle The triangle.
 * @param nCull The windings to discard.
 * @param pflT Receives the parameter of the point where the line meets the plane, 0 at the start
 *             and 1 at the end. A face discarded by winding leaves it unchanged.
 * @return Whether the segment passes through the triangle.
 * @ghidraAddress NTSC-U/C: 0x00290878
 * @ghidraAddress PAL: 0x0029a240
 */
bool IntersectSegmentTriangle(const Segment &segment,
                              const Triangle &triangle,
                              CullMode nCull,
                              float *pflT);

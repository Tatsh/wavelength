#pragma once

#include "math/plane.h"
#include "math/vector3.h"
#include "os/prnstream.h"

/**
 * Bounding sphere.
 *
 * The class is not polymorphic and emits no RTTI descriptor, so the name is inferred. Its text
 * dump writes the two members under the labels "\n\tcenter:" and " radius:".
 *
 * The centre is quadword aligned on the PlayStation 2, which gives the whole structure 16-byte
 * alignment and a size of 0x20 bytes. Rnd::Mesh::CopyFrom() relies on that size and moves the
 * sphere as two quadwords.
 */
class Sphere {
public:
    Vector3 mCenter; // +0x00
    float mRadius;   // +0x10
};

/**
 * Write a sphere's centre and radius on a tab-indented line.
 *
 * @param stream The stream to write to.
 * @param sphere The sphere.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00290670
 * @ghidraAddress PAL: 0x0029a038
 */
PrnStream &operator<<(PrnStream &stream, const Sphere &sphere);

/**
 * Report whether the line through a segment passes within a sphere.
 *
 * The parameter of the point of the line nearest the centre is reported without clamping it to
 * the segment.
 *
 * @param segment The segment.
 * @param sphere The sphere.
 * @param pflT Receives the parameter of the nearest point, 0 at the start and 1 at the end.
 * @return Whether the nearest point lies within the radius.
 * @ghidraAddress NTSC-U/C: 0x00290b38
 * @ghidraAddress PAL: 0x0029a500
 */
bool IntersectSegmentSphere(const Segment &segment, const Sphere &sphere, float *pflT);

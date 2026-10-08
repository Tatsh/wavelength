#pragma once

#include "math/plane.h"
#include "os/prnstream.h"

namespace Rnd {
class Dbg;
} // namespace Rnd
class Sphere;

/**
 * Six-plane view volume.
 *
 * The class is not polymorphic and emits no RTTI descriptor, so the name is inferred. The member
 * order and the member titles both come from the text writer, which walks six consecutive
 * quadwords and writes them under "\n\tfront:", "\n\tback:", "\n\tleft:", "\n\tright:",
 * "\n\ttop:", and "\n\tbottom:".
 */
class Frustum {
public:
    /**
     * Build the six planes of a perspective view volume.
     *
     * Every plane is expressed in camera space. The four side planes pass through the origin unless
     * flFov is zero. A zero flFov gives each side plane the distance of the unit offset it is built
     * through.
     *
     * @param flNear Distance to the near plane.
     * @param flFar Distance to the far plane.
     * @param flFov Field of view in radians.
     * @param flAspect Vertical extent divided by the horizontal extent.
     * @ghidraAddress NTSC-U/C: 0x00290c48
     * @ghidraAddress PAL: 0x0029a610
     */
    void Set(float flNear, float flFar, float flFov, float flAspect);

    Plane mFront;  // +0x00
    Plane mBack;   // +0x10
    Plane mLeft;   // +0x20
    Plane mRight;  // +0x30
    Plane mTop;    // +0x40
    Plane mBottom; // +0x50
};

/**
 * Write the six planes of a view volume.
 *
 * Each plane goes on its own tab-indented line under its title.
 *
 * @param stream The stream to write to.
 * @param frustum The view volume.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002904e0
 * @ghidraAddress PAL: 0x00299ea8
 */
PrnStream &operator<<(PrnStream &stream, const Frustum &frustum);

/**
 * Write the six planes of a view volume to a diagnostic sink.
 *
 * Each plane goes on its own tab-indented line under its title, as `(a: b: c: d:)` with two
 * decimals. Rnd::Cam::DumpText() is the one caller.
 *
 * @param sink The sink to write to.
 * @param frustum The view volume.
 * @return The sink.
 * @ghidraAddress NTSC-U/C: 0x0054f798
 * @ghidraAddress PAL: 0x0058fdd8
 */
Rnd::Dbg &operator<<(Rnd::Dbg &sink, const Frustum &frustum);

/** Bit of the VU0 status flag IsSphereOutsideFrustum() reports, the sticky sign flag. */
constexpr int kVu0StatusStickySign = 0x80;

/**
 * Report whether a sphere lies wholly outside at least one plane of a view volume.
 *
 * Each plane is dotted with the centre and the radius added, on VU0 in macro mode, and the sticky
 * sign bit of the status flag collects a negative sum from every plane. The bit is returned as it
 * stands rather than as a truth value.
 *
 * @param sphere The sphere, in the space the planes are expressed in.
 * @param frustum The view volume.
 * @return kVu0StatusStickySign when the whole sphere is behind some plane, and zero otherwise.
 * @ghidraAddress NTSC-U/C: 0x00290f90
 * @ghidraAddress PAL: 0x0029a958
 */
int IsSphereOutsideFrustum(const Sphere &sphere, const Frustum &frustum);

/**
 * Report whether a point lies on the inner side of every plane of a view volume.
 *
 * @param point The point, in the space the planes are expressed in.
 * @param frustum The view volume.
 * @return Whether no plane puts the point behind it.
 * @ghidraAddress NTSC-U/C: 0x00291090
 * @ghidraAddress PAL: 0x0029aa58
 */
bool IsPointInsideFrustum(const Vector3 &point, const Frustum &frustum);

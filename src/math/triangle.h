#pragma once

#include "math/plane.h"
#include "math/vector3.h"

/**
 * Triangle prepared for a segment test, as a vertex, the two edges from it, and their normal.
 *
 * The structure is not polymorphic and emits no RTTI descriptor, so the name and the member titles
 * are inferred.
 */
struct Triangle {
    Vector3 mOrigin; /*!< The first vertex. */
    Vector3 mEdge1;  /*!< The second vertex less the first. */
    Vector3 mEdge2;  /*!< The third vertex less the first. */
    Vector3 mNormal; /*!< The cross product of the two edges. */
};

/**
 * Test a segment against a triangle.
 *
 * A cull mode of 2 tests both faces. Otherwise the segment must approach the face the cull mode
 * keeps. The name is inferred.
 *
 * @param segment The segment, in the space of the triangle.
 * @param triangle The triangle.
 * @param nCull The RndMat::Cull of the material.
 * @param fT Receives the position of the strike along the segment, 0 at the start and 1 at the
 *        end. It is set whenever the facing test passes.
 * @return Whether the segment strikes the triangle.
 * @ghidraAddress NTSC-U/C: 0x00290878
 * @ghidraAddress PAL: 0x0029a240
 */
bool Intersect(const Segment &segment, const Triangle &triangle, int nCull, float &fT);

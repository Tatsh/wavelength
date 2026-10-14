#pragma once

#include "os/prnstream.h"

/**
 * Rotation quaternion, stored as the vector part followed by the scalar part.
 *
 * The class is not polymorphic and emits no RTTI descriptor, and no text dump writes component
 * labels for it. The component order is recovered rather than assumed, and three routines agree
 * on it. Set() from a matrix writes half the square root of one plus the matrix trace to the fourth
 * word and the three antisymmetric differences of the off-diagonal terms to the first three. Set()
 * from an axis and an angle writes the cosine of the half angle to the fourth word and the axis
 * scaled by the sine of the half angle to the first three. QuatMultiply() forms the Hamilton
 * product with the fourth word as the real part.
 *
 * The type is a full quadword of meaningful data rather than a padded triple. The four-term dot
 * product in QuatSlerp() proves that.
 */
class Quat {
public:
    /**
     * Convert a rotation matrix to this quaternion.
     *
     * A positive trace takes the direct route. Otherwise the largest diagonal term selects which
     * component of the vector part is recovered first, and the other two components and the
     * scalar part follow from it. The routine is the Shoemake construction, including the cyclic
     * successor table.
     *
     * @param pMat3Rows The rotation, three rows of four floats, treated as orthonormal.
     * @return This quaternion.
     * @ghidraAddress NTSC-U/C: 0x00292710
     * @ghidraAddress PAL: 0x0029c0d8
     */
    Quat &Set(const float *pMat3Rows);

    /**
     * Set this quaternion to a rotation about an axis.
     *
     * The axis is used as supplied and is not normalised.
     *
     * @param pAxis The rotation axis, three floats.
     * @param flAngle The rotation angle in radians.
     * @return This quaternion.
     */
    Quat &Set(const float *pAxis, float flAngle);

    float x;
    float y;
    float z;
    float w;
};

/**
 * Write a quaternion as `x: y: z: w:`.
 *
 * @param stream The stream to write to.
 * @param quat The quaternion.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00292d88
 * @ghidraAddress PAL: 0x0029c750
 */
PrnStream &operator<<(PrnStream &stream, const Quat &quat);

namespace Rnd {} // namespace Rnd

/**
 * Build a rotation quaternion from three Euler angles.
 *
 * The composition is the same one Rnd::MakeRotMatrix() produces from three angles. That is, the
 * product is qz * qx * qy, with each factor a rotation about one axis by the matching component.
 * The sines and cosines come from SinApprox().
 *
 * @param pAngles The three angles in radians, ordered X, Y, and Z.
 * @return The quaternion.
 * @ghidraAddress NTSC-U/C: 0x002925d8
 * @ghidraAddress PAL: 0x0029bfa0
 */
Quat EulerAnglesToQuat(const float *pAngles);

/**
 * Interpolate between two rotation quaternions along the shorter arc.
 *
 * A negative dot product flips the second quaternion. The interpolation therefore never takes
 * the long way round. Both endpoint values of the parameter are special cased and copy an
 * endpoint verbatim. Two inputs whose dot product is within 1e-5 of one fall back to a
 * component-wise linear blend. The sines come from SinApprox().
 *
 * @param from The rotation at a parameter of zero.
 * @param to The rotation at a parameter of one.
 * @param out Receives the interpolation.
 * @param flT The interpolation parameter.
 * @ghidraAddress NTSC-U/C: 0x00292970
 * @ghidraAddress PAL: 0x0029c338
 */
void QuatSlerp(const Quat &from, const Quat &to, Quat &out, float flT);

namespace Rnd {

/**
 * Convert a rotation quaternion to a rotation matrix.
 *
 * The result is the transpose of the usual column-vector rotation, because this engine transforms
 * row vectors. Only nine of the twelve words are written, and the fourth word of each row is
 * untouched.
 *
 * @param quat The rotation, treated as unit length.
 * @param pMat3Rows Receives the rotation, three rows of four floats.
 * @ghidraAddress NTSC-U/C: 0x00292ce0
 * @ghidraAddress PAL: 0x0029c6a8
 */
void MakeRotMatrix(const Quat &quat, float *pMat3Rows);

} // namespace Rnd

/**
 * Report the scale each basis row carries.
 *
 * Each scale is the length of its row. The Z scale is negated unless the dot product of the Z row
 * with the cross product of the X and Y rows is positive.
 *
 * @param pMat3Rows The basis, three rows of four floats.
 * @param pScale Receives the three scales.
 * @ghidraAddress NTSC-U/C: 0x002924f0
 * @ghidraAddress PAL: 0x0029beb8
 */
void Mat33ExtractScale(const float *pMat3Rows, float *pScale);

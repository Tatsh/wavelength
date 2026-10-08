#pragma once

#include "math/vector3.h"
#include "os/prnstream.h"

/**
 * Affine transform stored as three basis rows and a translation row.
 *
 * The class is not polymorphic and emits no RTTI descriptor, so the name is inferred. The four
 * rows are quadwords the vector unit multiplies in place, which is why each row is a padded
 * Vector3 rather than a bare triple. Rnd::MatStage and Rnd::Transformable both store transforms
 * in this shape, and the identity every default constructor writes is the three unit basis rows
 * followed by a zero translation.
 */
struct Transform {
    Vector3 mBasisX;      // +0x00
    Vector3 mBasisY;      // +0x10
    Vector3 mBasisZ;      // +0x20
    Vector3 mTranslation; // +0x30
};

/**
 * Concatenate two transforms, applying the second and then the first.
 *
 * The result may not alias the second transform. The name is inferred.
 *
 * @param out Receives the concatenation.
 * @param first The transform applied last, such as a parent's world transform.
 * @param second The transform applied first, such as a child's local transform.
 * @ghidraAddress NTSC-U/C: 0x00293410
 * @ghidraAddress PAL: 0x0029cdd8
 */
void Multiply(Transform &out, const Transform &first, const Transform &second);

/**
 * Invert a transform whose basis is orthonormal.
 *
 * The basis is transposed and the translation is rotated back and negated. The name is
 * inferred.
 *
 * @param out Receives the inverse. It may not alias the source.
 * @param xfm The transform.
 * @ghidraAddress NTSC-U/C: 0x00293488
 * @ghidraAddress PAL: 0x0029ce50
 */
void Invert(Transform &out, const Transform &xfm);

/**
 * Report the length of each basis row, negating the third when the basis is left handed.
 *
 * The name is inferred.
 *
 * @param xfm The transform.
 * @param scale Receives the three lengths.
 * @ghidraAddress NTSC-U/C: 0x002924f0
 * @ghidraAddress PAL: 0x0029beb8
 */
void MakeScale(const Transform &xfm, Vector3 &scale);

/**
 * Write the four rows of a transform.
 *
 * @param stream The stream to write to.
 * @param xfm The transform.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002930a0
 * @ghidraAddress PAL: 0x0029ca68
 */
PrnStream &operator<<(PrnStream &stream, const Transform &xfm);

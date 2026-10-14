#pragma once

extern "C" {

/**
 * Invert a rigid transform.
 *
 * Sony libvu0, linked as shipped. The basis rows are transposed and the translation row becomes the
 * negated transposed basis applied to the original translation. An input whose basis is not
 * orthonormal therefore does not invert correctly.
 *
 * @param pDst Receives the inverse, four rows of four floats.
 * @param pSrc The transform to invert, four rows of four floats.
 * @ghidraAddress NTSC-U/C: 0x00293488
 * @ghidraAddress PAL: 0x0029ce50
 */
void InversMatrix(float *pDst, const float *pSrc);

/**
 * Multiply two affine transforms across all four words of every row.
 *
 * A member of the Sony libvu0 object, linked as shipped, whose published name is unknown. It is
 * sceVu0MulMatrix with the loop cut to three rows that ignore the fourth word of pB, and a fourth
 * row that adds the translation row of pA. Callers use it where the fourth word is consumed.
 *
 * @param pDst Receives the product, four rows of four floats. It may alias either factor.
 * @param pA The left factor, four rows of four floats.
 * @param pB The right factor, four rows of four floats.
 * @ghidraAddress NTSC-U/C: 0x002933b0
 * @ghidraAddress PAL: 0x0029cd78
 */
void sceVu0MulAffineMatrix(float *pDst, const float *pA, const float *pB);

/**
 * Multiply two affine transforms in the first three words of every row.
 *
 * A member of the Sony libvu0 object, linked as shipped, whose published name is unknown. It is
 * sceVu0MulAffineMatrix under an `xyz` destination mask. The fourth word of every destination row
 * therefore receives whatever the vector register already had.
 *
 * @param pDst Receives the product, four rows of four floats. It may alias either factor.
 * @param pA The left factor, four rows of four floats.
 * @param pB The right factor, four rows of four floats.
 * @ghidraAddress NTSC-U/C: 0x00293410
 * @ghidraAddress PAL: 0x0029cdd8
 */
void sceVu0MulAffineMatrixXyz(float *pDst, const float *pA, const float *pB);

} // extern "C"

namespace Rnd {

/**
 * Build a rotation matrix from three Euler angles.
 *
 * The product is Ry * Rx * Rz, with each factor a rotation about one axis by the matching
 * component and every factor written for row vectors. Mat34DecomposeEulerScale() inverts the same
 * composition and EulerAnglesToQuat() reproduces it as a quaternion. Only nine of the twelve words
 * are written, and the fourth word of each row is untouched. The sines and cosines come from
 * SinApprox().
 *
 * @param pAngles The three angles in radians, ordered X, Y, and Z.
 * @param pMat3Rows Receives the rotation, three rows of four floats.
 * @ghidraAddress NTSC-U/C: 0x00292b38
 * @ghidraAddress PAL: 0x0029c500
 */
void MakeRotMatrix(const float *pAngles, float *pMat3Rows);

} // namespace Rnd

/**
 * Build an orthonormal basis around one axis and a reference direction.
 *
 * The Y row is the supplied axis normalised. The X row is that axis crossed with the reference
 * direction and normalised. The Z row is X crossed with Y, and needs no normalising. A reference
 * direction parallel to the axis yields a zero cross product and then a division by zero.
 *
 * Each row is written as a whole quadword, and the two cross products propagate the fourth word of
 * the supplied axis into the other two rows. All three rows therefore end with pAxisY[3]. The 1.0
 * that the inlined vector construction writes into the scratch quadword is overwritten before
 * anything reads it.
 *
 * @param pAxisY The direction the Y row takes, four floats.
 * @param pReference The reference direction, three floats.
 * @param pMat3Rows Receives the basis, three rows of four floats.
 * @ghidraAddress NTSC-U/C: 0x00292c60
 * @ghidraAddress PAL: 0x0029c628
 */
void Mat33BuildOrthonormal(const float *pAxisY, const float *pReference, float *pMat3Rows);

namespace Rnd {

/**
 * Scale each row of a rotation matrix by one component of a vector.
 *
 * Row zero takes the first component, row one the second, and row two the third. This is how a
 * non-uniform scale is folded into a basis. Only nine of the twelve words are written, and the
 * fourth word of each row is untouched.
 *
 * @param pScale The three scales.
 * @param pMat3Rows The basis to scale, three rows of four floats.
 * @param pOut Receives the scaled basis, and may alias pMat3Rows.
 */
void Scale(const float *pScale, const float *pMat3Rows, float *pOut);

} // namespace Rnd

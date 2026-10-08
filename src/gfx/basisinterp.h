#pragma once

#include "math/transform.h"

/**
 * Blend the basis of one transform towards another through their quaternions.
 *
 * Only the three basis rows of pOut are written. The name is inferred.
 *
 * @param from The basis at a factor of 0.
 * @param to The basis at a factor of 1.
 * @param flT The factor.
 * @param pOut Receives the blended basis, and may alias either input.
 * @ghidraAddress NTSC-U/C: 0x001e2b38
 * @ghidraAddress PAL: 0x001eb8d8
 */
void InterpBasis(const Transform &from, const Transform &to, float flT, Transform *pOut);

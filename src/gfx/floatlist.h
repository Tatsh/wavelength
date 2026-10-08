#pragma once

#include <vector>

#include "script/dataarray.h"

/**
 * Replace a list of numbers with the nodes of an array after its tag.
 *
 * The name is inferred.
 *
 * @param values Receives one number for each node after the first.
 * @param pData The array.
 * @ghidraAddress NTSC-U/C: 0x001e31f0
 * @ghidraAddress PAL: 0x001ebf90
 */
void LoadFloatList(std::vector<float> &values, const DataArray *pData);

/**
 * Evaluate a polynomial by Horner's rule.
 *
 * The name is inferred.
 *
 * @param coefficients The coefficients, the highest power first.
 * @param flX The variable.
 * @return The value, 0 for an empty list.
 * @ghidraAddress NTSC-U/C: 0x001e32c8
 * @ghidraAddress PAL: 0x001ec068
 */
float EvalPolynomial(const std::vector<float> &coefficients, float flX);

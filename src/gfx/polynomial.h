#pragma once

#include <vector>

#include "script/dataarray.h"

/**
 * A polynomial read from a configuration array.
 *
 * The class is not polymorphic, and the name is inferred from Eval(). The object is the vector of
 * its coefficients.
 */
class Polynomial {
public:
    /**
     * Construct a polynomial with no coefficients.
     *
     * @ghidraAddress NTSC-U/C: 0x001e3170
     * @ghidraAddress PAL: 0x001ebf10
     */
    Polynomial();

    /**
     * Destroy the polynomial.
     *
     * @ghidraAddress NTSC-U/C: 0x001e3190
     * @ghidraAddress PAL: 0x001ebf30
     */
    ~Polynomial();

    /**
     * Replace the coefficients with the nodes of an array after its tag.
     *
     * @param pData The array, the coefficient of the highest power first.
     * @ghidraAddress NTSC-U/C: 0x001e31f0
     * @ghidraAddress PAL: 0x001ebf90
     */
    void Load(const DataArray *pData);

    /**
     * Evaluate the polynomial by Horner's rule.
     *
     * @param flX The variable.
     * @return The value, 0 for a polynomial with no coefficients.
     * @ghidraAddress NTSC-U/C: 0x001e32c8
     * @ghidraAddress PAL: 0x001ec068
     */
    float Eval(float flX) const;

private:
    std::vector<float> mCoefficients; /*!< The coefficients, the highest power first. */
};

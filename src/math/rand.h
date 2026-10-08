#pragma once

/**
 * Generator of pseudo-random numbers with a lagged Fibonacci sequence of 256 words.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is 0x408 bytes.
 * Only the members GameLogic uses are declared.
 */
class Rand {
public:
    /**
     * Construct a generator and seed it.
     *
     * @param nSeed The seed.
     * @ghidraAddress NTSC-U/C: 0x00292230
     * @ghidraAddress PAL: 0x0029bbf8
     */
    explicit Rand(int nSeed);

    /**
     * Draw an integer.
     *
     * @param nLow The smallest value.
     * @param nHigh One more than the largest value.
     * @return The integer.
     * @ghidraAddress NTSC-U/C: 0x002922b8
     * @ghidraAddress PAL: 0x0029bc80
     */
    int Int(int nLow, int nHigh);
};

/**
 * Draw an integer from the shared generator.
 *
 * @param nLow The smallest value.
 * @param nHigh One more than the largest value.
 * @return The integer.
 * @ghidraAddress NTSC-U/C: 0x00292428
 * @ghidraAddress PAL: 0x0029bdf0
 */
int RandomInt(int nLow, int nHigh);

/**
 * Draw a floating-point number from the shared generator.
 *
 * @param fLow The smallest value.
 * @param fHigh The largest value.
 * @return The number.
 * @ghidraAddress NTSC-U/C: 0x00292470
 * @ghidraAddress PAL: 0x0029be38
 */
float RandomFloat(float fLow, float fHigh);

/**
 * Draw a floating-point number from 0 to 1 from the shared generator.
 *
 * @return The number.
 * @ghidraAddress NTSC-U/C: 0x00292450
 * @ghidraAddress PAL: 0x0029be18
 */
float RandomFloat();

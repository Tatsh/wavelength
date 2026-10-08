#pragma once

/**
 * Generator of pseudo-random numbers with a lagged Fibonacci sequence of 256 words.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is 0x408 bytes.
 */
class Rand {
public:
    /** Words of the state table. */
    static constexpr int kTableSize = 256;

    /**
     * Construct a generator and seed it.
     *
     * @param nSeed The seed.
     * @ghidraAddress NTSC-U/C: 0x00292230
     * @ghidraAddress PAL: 0x0029bbf8
     */
    explicit Rand(int nSeed);

    /**
     * Fill the state table from a linear congruential sequence and reset both indices.
     *
     * @param nSeed The starting value of the sequence.
     * @ghidraAddress NTSC-U/C: 0x00292260
     * @ghidraAddress PAL: 0x0029bc28
     */
    void Seed(int nSeed);

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

    /**
     * Draw a floating-point number.
     *
     * @param fLow The smallest value.
     * @param fHigh The largest value.
     * @return The number.
     * @ghidraAddress NTSC-U/C: 0x00292300
     * @ghidraAddress PAL: 0x0029bcc8
     */
    float Float(float fLow, float fHigh);

    /**
     * Draw a floating-point number in [0, 1).
     *
     * @return The low sixteen bits of a draw divided by 65536.
     * @ghidraAddress NTSC-U/C: 0x00292340
     * @ghidraAddress PAL: 0x0029bd08
     */
    float Float();

    /**
     * Draw the next word.
     *
     * Both indices wrap to zero on arriving at 249, so the draws cycle through the first 249 words.
     *
     * @return The replaced word.
     * @ghidraAddress NTSC-U/C: 0x00292378
     * @ghidraAddress PAL: 0x0029bd40
     */
    int Int();

    int mIndex;             /*!< The word the next draw replaces. */
    int mLagIndex;          /*!< The word the next draw combines with the replaced one. */
    int mTable[kTableSize]; /*!< The state. */
};

/**
 * Reseed the shared generator.
 *
 * @param nSeed The seed.
 * @ghidraAddress NTSC-U/C: 0x002923e0
 * @ghidraAddress PAL: 0x0029bda8
 */
void SeedRand(int nSeed);

/**
 * Draw a word from the shared generator.
 *
 * @return The word.
 * @ghidraAddress NTSC-U/C: 0x00292408
 * @ghidraAddress PAL: 0x0029bdd0
 */
int RandomWord();

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
 * Draw a floating-point number in [0, 1) from the shared generator.
 *
 * @return The number.
 * @ghidraAddress NTSC-U/C: 0x00292450
 * @ghidraAddress PAL: 0x0029be18
 */
float RandomFraction();

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

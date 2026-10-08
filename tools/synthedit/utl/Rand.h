#pragma once

/**
 * Random number generator with a table of 256 words.
 *
 * The RTTI records the class. The object is 0x408 bytes. Seed() fills the table from a linear
 * congruential sequence and sets the two read positions 103 words apart, the R250 arrangement.
 * The control seeds the generator and never draws from it.
 */
class Rand {
public:
    /**
     * Seed a new generator.
     *
     * @param seed The seed.
     * @ghidraAddress 0x1001bcf0
     */
    Rand(int seed);

    /**
     * Refill the table.
     *
     * @param seed The seed.
     * @ghidraAddress 0x1001bd10
     */
    void Seed(int seed);

private:
    enum { kTableSize = 256 };

    int mIndex1;            /*!< The first read position. */
    int mIndex2;            /*!< The second read position. */
    int mTable[kTableSize]; /*!< The generator's words. */
};

/**
 * Seed the shared generator.
 *
 * @param seed The seed.
 * @ghidraAddress 0x1001bd80
 */
void SeedRand(int seed);

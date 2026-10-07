#pragma once

/**
 * A linear congruential random number generator.
 *
 * The class has no RTTI, and its name is inferred from its role. Every member is static. The
 * synthesiser seeds it and never draws from it.
 */
class Random {
public:
    /**
     * Seed the generator.
     *
     * @param seed The seed.
     * @ghidraAddress NTSC-U/C: 0x000060f0
     * @ghidraAddress PAL: 0x000060f0
     */
    static void Seed(unsigned int seed);

    /**
     * Draw a number from a range, advancing the generator two steps.
     *
     * @param low Smallest result.
     * @param high One past the largest result.
     * @return A number from @p low up to but not including @p high.
     * @ghidraAddress NTSC-U/C: 0x000060fc
     * @ghidraAddress PAL: 0x000060fc
     */
    static int Range(int low, int high);

private:
    static unsigned int sSeed; /*!< The generator's state. */
};

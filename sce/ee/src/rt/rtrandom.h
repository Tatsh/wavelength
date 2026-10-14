#ifndef RTRANDOM_H
#define RTRANDOM_H

#ifdef __cplusplus
extern "C" {
#endif

/** The Mersenne Twister generator of the SCE-RT library, which its keys come from. */

/**
 * Seed the generator from one value.
 *
 * @param uSeed The seed. Its lowest bit is forced on.
 * @ghidraAddress NTSC-U/C: 0x002aa6e0
 * @ghidraAddress PAL: 0x002b4650
 */
void RtSeedRandom(unsigned int uSeed);

/**
 * Seed the generator from an array of values.
 *
 * @param puKey The values.
 * @param uCount The number of values. At most 623 are used.
 * @ghidraAddress NTSC-U/C: 0x002aa728
 * @ghidraAddress PAL: 0x002b4698
 */
void RtSeedRandomFromArray(const unsigned int *puKey, unsigned int uCount);

/**
 * Regenerate the state of the generator, seeding it first when it never was.
 *
 * @ghidraAddress NTSC-U/C: 0x002aa78c
 * @ghidraAddress PAL: 0x002b46fc
 */
void RtRegenerateRandom(void);

/**
 * Draw the next value.
 *
 * @return The value.
 * @ghidraAddress NTSC-U/C: 0x002aa908
 * @ghidraAddress PAL: 0x002b4878
 */
unsigned int RtRandom(void);

/**
 * Seed the generator from a value mixed with the clocks and the timers of the console.
 *
 * @param uSeed The value.
 * @ghidraAddress NTSC-U/C: 0x002aa990
 * @ghidraAddress PAL: 0x002b4900
 */
void RtSeedRandomFromEntropy(unsigned int uSeed);

#ifdef __cplusplus
}
#endif

#endif

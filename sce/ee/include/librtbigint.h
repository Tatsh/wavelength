#ifndef LIBRTBIGINT_H
#define LIBRTBIGINT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Unsigned 1024-bit integer arithmetic of the SCE-RT network library, used for its RSA keys.
 *
 * The routine names take the prefix of the one original name the library records,
 * lintGetBits(). The other names and the parameter names are inferred.
 */

/** The number of 32-bit words of a large integer. */
#define RT_LARGE_INT_LIMB_COUNT 32

/** The number of bits of a large integer. */
#define RT_LARGE_INT_BITS 1024

/** An unsigned large integer. */
typedef struct {
    unsigned int limbs[RT_LARGE_INT_LIMB_COUNT]; /*!< The 32-bit words, least significant first. */
} rtLargeInt;

/**
 * Set a large integer to a 32-bit value.
 *
 * @param value Receives the integer.
 * @param word The value.
 * @ghidraAddress NTSC-U/C: 0x002edc18
 * @ghidraAddress PAL: 0x002f7b88
 */
void lintSet(rtLargeInt *value, unsigned int word);

/**
 * Report whether a large integer is zero.
 *
 * @param value The integer.
 * @return 1 when the integer is zero, otherwise 0.
 * @ghidraAddress NTSC-U/C: 0x002edce4
 * @ghidraAddress PAL: 0x002f7c54
 */
int lintIsZero(const rtLargeInt *value);

/**
 * Report whether a large integer is one.
 *
 * @param value The integer.
 * @return 1 when the integer is one, otherwise 0.
 * @ghidraAddress NTSC-U/C: 0x002edd1c
 * @ghidraAddress PAL: 0x002f7c8c
 */
int lintIsOne(const rtLargeInt *value);

/**
 * Compute the difference of two large integers.
 *
 * @param minuend The integer to subtract from.
 * @param subtrahend The integer to subtract.
 * @return The difference, modulo 2 to the power of RT_LARGE_INT_BITS.
 * @ghidraAddress NTSC-U/C: 0x002eded0
 * @ghidraAddress PAL: 0x002f7e40
 */
rtLargeInt lintDifference(const rtLargeInt *minuend, const rtLargeInt *subtrahend);

/**
 * Divide two large integers.
 *
 * @param dividend The integer to divide.
 * @param divisor The integer to divide by.
 * @param quotient Receives the quotient.
 * @param remainder Receives the remainder.
 * @ghidraAddress NTSC-U/C: 0x002ee1c8
 * @ghidraAddress PAL: 0x002f8138
 */
void lintDivide(const rtLargeInt *dividend,
                const rtLargeInt *divisor,
                rtLargeInt *quotient,
                rtLargeInt *remainder);

/**
 * Generate a random probable prime.
 *
 * The prime has the requested number of bits, with its two highest bits set.
 *
 * @param nBits The number of bits.
 * @return The prime.
 * @ghidraAddress NTSC-U/C: 0x002eeddc
 * @ghidraAddress PAL: 0x002f8d4c
 */
rtLargeInt lintGeneratePrime(unsigned int nBits);

/**
 * Raise a large integer to a power modulo an odd modulus with Montgomery multiplication.
 *
 * A zero modulus fails an assertion and returns the modulus. An even modulus fails an assertion
 * and falls back to plain modular exponentiation.
 *
 * @param base The base.
 * @param exponent The exponent.
 * @param modulus The modulus.
 * @return The power modulo the modulus.
 * @ghidraAddress NTSC-U/C: 0x002ef0c0
 * @ghidraAddress PAL: 0x002f9030
 */
rtLargeInt
lintMontgomeryModExp(const rtLargeInt *base, const rtLargeInt *exponent, const rtLargeInt *modulus);

// Routines of the bignum core that the bignum core headers do not declare yet.

/**
 * Count the significant bits of a large integer.
 *
 * @param value The integer.
 * @return The index of the highest set bit plus one, or 0 for zero.
 * @ghidraAddress NTSC-U/C: 0x002be244
 * @ghidraAddress PAL: 0x002c81b4
 */
unsigned int lintGetBits(const rtLargeInt *value);

/**
 * Compare two large integers.
 *
 * @param left The first integer.
 * @param right The second integer.
 * @return 1 when the first integer is greater than or equal to the second, otherwise 0.
 * @ghidraAddress NTSC-U/C: 0x002be3bc
 * @ghidraAddress PAL: 0x002c832c
 */
int lintIsGreaterOrEqual(const rtLargeInt *left, const rtLargeInt *right);

/**
 * Shift a large integer right.
 *
 * @param value The integer.
 * @param count The number of bits.
 * @ghidraAddress NTSC-U/C: 0x002be43c
 * @ghidraAddress PAL: 0x002c83ac
 */
void lintShiftRight(rtLargeInt *value, unsigned int count);

/**
 * Shift a large integer left.
 *
 * @param value The integer.
 * @param count The number of bits.
 * @ghidraAddress NTSC-U/C: 0x002be574
 * @ghidraAddress PAL: 0x002c84e4
 */
void lintShiftLeft(rtLargeInt *value, unsigned int count);

/**
 * Add a large integer to another.
 *
 * @param sum The integer to add to.
 * @param addend The integer to add.
 * @ghidraAddress NTSC-U/C: 0x002be6c0
 * @ghidraAddress PAL: 0x002c8630
 */
void lintAdd(rtLargeInt *sum, const rtLargeInt *addend);

/**
 * Add a large integer to another over the words that a bit count covers.
 *
 * @param sum The integer to add to.
 * @param addend The integer to add.
 * @param nBits The number of bits. The routine adds nBits / 32 + 1 words, at most all of them.
 * @ghidraAddress NTSC-U/C: 0x002be744
 * @ghidraAddress PAL: 0x002c86b4
 */
void lintAddBits(rtLargeInt *sum, const rtLargeInt *addend, unsigned int nBits);

/**
 * Subtract a large integer from another.
 *
 * @param difference The integer to subtract from.
 * @param subtrahend The integer to subtract.
 * @ghidraAddress NTSC-U/C: 0x002be7e0
 * @ghidraAddress PAL: 0x002c8750
 */
void lintSubtract(rtLargeInt *difference, const rtLargeInt *subtrahend);

/**
 * Subtract a large integer from another over the words that a bit count covers.
 *
 * @param difference The integer to subtract from.
 * @param subtrahend The integer to subtract.
 * @param nBits The number of bits. The routine subtracts nBits / 32 + 1 words, at most all of
 * them.
 * @ghidraAddress NTSC-U/C: 0x002be864
 * @ghidraAddress PAL: 0x002c87d4
 */
void lintSubtractBits(rtLargeInt *difference, const rtLargeInt *subtrahend, unsigned int nBits);

/**
 * Reduce a large integer modulo another.
 *
 * @param value The integer, which receives the remainder.
 * @param modulus The modulus.
 * @ghidraAddress NTSC-U/C: 0x002be900
 * @ghidraAddress PAL: 0x002c8870
 */
void lintMod(rtLargeInt *value, const rtLargeInt *modulus);

/**
 * Multiply two large integers, truncating the product to RT_LARGE_INT_BITS.
 *
 * The product may be either factor.
 *
 * @param product Receives the product.
 * @param left The first factor.
 * @param right The second factor.
 * @ghidraAddress NTSC-U/C: 0x002bed2c
 * @ghidraAddress PAL: 0x002c8c9c
 */
void lintMultiply(rtLargeInt *product, const rtLargeInt *left, const rtLargeInt *right);

/**
 * Multiply two large integers modulo a third.
 *
 * The product may be either factor.
 *
 * @param product Receives the product modulo the modulus.
 * @param left The first factor.
 * @param right The second factor.
 * @param modulus The modulus.
 * @ghidraAddress NTSC-U/C: 0x002beef4
 * @ghidraAddress PAL: 0x002c8e64
 */
void lintMultiplyMod(rtLargeInt *product,
                     const rtLargeInt *left,
                     const rtLargeInt *right,
                     const rtLargeInt *modulus);

/**
 * Draw the next 32-bit value of the random number generator of the library.
 *
 * @return The value.
 * @ghidraAddress NTSC-U/C: 0x002aa908
 * @ghidraAddress PAL: 0x002b4878
 */
unsigned int rtDmeRandom(void);

/**
 * Seed the random number generator of the library from a value mixed with gathered entropy.
 *
 * @param seed The value.
 * @ghidraAddress NTSC-U/C: 0x002aa990
 * @ghidraAddress PAL: 0x002b4900
 */
void rtDmeSeedRandom(unsigned int seed);

#ifdef __cplusplus
}
#endif

#endif

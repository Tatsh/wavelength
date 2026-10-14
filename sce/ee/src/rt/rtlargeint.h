#ifndef RTLARGEINT_H
#define RTLARGEINT_H

#include <librtdme.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Unsigned integers of 1024 bits for the RSA keys of the SCE-RT library. The words are stored
 * least significant first.
 */

/** The words of a large integer. */
#define RT_LARGE_INT_WORDS 32

/** The bits of a large integer. */
#define RT_LARGE_INT_BITS 1024

/** The size of a large integer in bytes. */
#define RT_LARGE_INT_SIZE 128

/** A key of 512 bits, half of a large integer. */
typedef rtNetKey RtKey;

/** A large integer. The arithmetic moves it a quadword at a time. */
typedef union {
    unsigned int auWords[RT_LARGE_INT_WORDS]; /*!< The words, least significant first. */
    unsigned char abBytes[RT_LARGE_INT_SIZE]; /*!< The bytes, least significant first. */
    RtKey aHalves[2];                         /*!< The low and high halves. */
} __attribute__((aligned(16))) RtLargeInt;

/**
 * Report the number of significant bits of an integer.
 *
 * @param pValue The integer.
 * @return The bits, 0 for zero.
 * @ghidraAddress NTSC-U/C: 0x002be244
 * @ghidraAddress PAL: 0x002c81b4
 */
unsigned int RtLargeIntBitLength(const RtLargeInt *pValue);

/**
 * Compare two integers.
 *
 * @param pLeft The left integer.
 * @param pRight The right integer.
 * @return Non-zero when the left integer is the greater.
 * @ghidraAddress NTSC-U/C: 0x002be2d4
 * @ghidraAddress PAL: 0x002c8244
 */
int RtLargeIntIsGreater(const RtLargeInt *pLeft, const RtLargeInt *pRight);

/**
 * Compare two integers.
 *
 * @param pLeft The left integer.
 * @param pRight The right integer.
 * @return Non-zero when the left integer is the smaller.
 * @ghidraAddress NTSC-U/C: 0x002be348
 * @ghidraAddress PAL: 0x002c82b8
 */
int RtLargeIntIsLess(const RtLargeInt *pLeft, const RtLargeInt *pRight);

/**
 * Compare two integers.
 *
 * @param pLeft The left integer.
 * @param pRight The right integer.
 * @return Non-zero when the left integer is the greater or they are equal.
 * @ghidraAddress NTSC-U/C: 0x002be3bc
 * @ghidraAddress PAL: 0x002c832c
 */
int RtLargeIntIsGreaterOrEqual(const RtLargeInt *pLeft, const RtLargeInt *pRight);

/**
 * Shift an integer right in place.
 *
 * @param pValue The integer.
 * @param uBits The bits to shift by, below 1024.
 * @ghidraAddress NTSC-U/C: 0x002be43c
 * @ghidraAddress PAL: 0x002c83ac
 */
void RtLargeIntShiftRight(RtLargeInt *pValue, unsigned int uBits);

/**
 * Shift an integer left in place, dropping the bits shifted past the top.
 *
 * @param pValue The integer.
 * @param uBits The bits to shift by, below 1024.
 * @ghidraAddress NTSC-U/C: 0x002be574
 * @ghidraAddress PAL: 0x002c84e4
 */
void RtLargeIntShiftLeft(RtLargeInt *pValue, unsigned int uBits);

/**
 * Add an integer to another in place, dropping the carry out of the top.
 *
 * @param pSum The integer added to.
 * @param pAddend The integer added.
 * @ghidraAddress NTSC-U/C: 0x002be6c0
 * @ghidraAddress PAL: 0x002c8630
 */
void RtLargeIntAdd(RtLargeInt *pSum, const RtLargeInt *pAddend);

/**
 * Add the low words of an integer to another in place.
 *
 * @param pSum The integer added to.
 * @param pAddend The integer added.
 * @param uBits The bits to add. One word past them takes the carry.
 * @ghidraAddress NTSC-U/C: 0x002be744
 * @ghidraAddress PAL: 0x002c86b4
 */
void RtLargeIntAddBits(RtLargeInt *pSum, const RtLargeInt *pAddend, unsigned int uBits);

/**
 * Subtract an integer from another in place, wrapping below zero.
 *
 * @param pDifference The integer subtracted from.
 * @param pSubtrahend The integer subtracted.
 * @ghidraAddress NTSC-U/C: 0x002be7e0
 * @ghidraAddress PAL: 0x002c8750
 */
void RtLargeIntSubtract(RtLargeInt *pDifference, const RtLargeInt *pSubtrahend);

/**
 * Subtract the low words of an integer from another in place.
 *
 * @param pDifference The integer subtracted from.
 * @param pSubtrahend The integer subtracted.
 * @param uBits The bits to subtract. One word past them takes the borrow.
 * @ghidraAddress NTSC-U/C: 0x002be864
 * @ghidraAddress PAL: 0x002c87d4
 */
void RtLargeIntSubtractBits(RtLargeInt *pDifference,
                            const RtLargeInt *pSubtrahend,
                            unsigned int uBits);

/**
 * Reduce an integer modulo another in place, by shifting and subtracting.
 *
 * @param pValue The integer reduced.
 * @param pModulus The modulus.
 * @ghidraAddress NTSC-U/C: 0x002be900
 * @ghidraAddress PAL: 0x002c8870
 */
void RtLargeIntModulo(RtLargeInt *pValue, const RtLargeInt *pModulus);

/**
 * Multiply two integers. The product keeps the words its operands need, up to 1024 bits.
 *
 * @param pProduct Receives the product.
 * @param pLeft The left factor.
 * @param pRight The right factor.
 * @ghidraAddress NTSC-U/C: 0x002bed2c
 * @ghidraAddress PAL: 0x002c8c9c
 */
void RtLargeIntMultiply(RtLargeInt *pProduct, const RtLargeInt *pLeft, const RtLargeInt *pRight);

/**
 * Multiply two integers modulo a third.
 *
 * @param pProduct Receives the product.
 * @param pLeft The left factor.
 * @param pRight The right factor.
 * @param pModulus The modulus.
 * @ghidraAddress NTSC-U/C: 0x002beef4
 * @ghidraAddress PAL: 0x002c8e64
 */
void RtLargeIntMultiplyModulo(RtLargeInt *pProduct,
                              const RtLargeInt *pLeft,
                              const RtLargeInt *pRight,
                              const RtLargeInt *pModulus);

/**
 * Set an integer to a small value.
 *
 * @param pValue The integer.
 * @param uValue The value.
 * @ghidraAddress NTSC-U/C: 0x002edc18
 * @ghidraAddress PAL: 0x002f7b88
 */
void RtLargeIntSet(RtLargeInt *pValue, unsigned int uValue);

/**
 * Test an integer for zero.
 *
 * @param pValue The integer.
 * @return Non-zero when the integer is zero.
 * @ghidraAddress NTSC-U/C: 0x002edce4
 * @ghidraAddress PAL: 0x002f7c54
 */
int RtLargeIntIsZero(const RtLargeInt *pValue);

/**
 * Test an integer for one.
 *
 * @param pValue The integer.
 * @return Non-zero when the integer is one.
 * @ghidraAddress NTSC-U/C: 0x002edd1c
 * @ghidraAddress PAL: 0x002f7c8c
 */
int RtLargeIntIsOne(const RtLargeInt *pValue);

/**
 * Subtract one integer from another into a third.
 *
 * @param pDifference Receives the difference.
 * @param pMinuend The integer subtracted from.
 * @param pSubtrahend The integer subtracted.
 * @ghidraAddress NTSC-U/C: 0x002eded0
 * @ghidraAddress PAL: 0x002f7e40
 */
void RtLargeIntDifference(RtLargeInt *pDifference,
                          const RtLargeInt *pMinuend,
                          const RtLargeInt *pSubtrahend);

/**
 * Divide one integer by another.
 *
 * @param pDividend The dividend.
 * @param pDivisor The divisor.
 * @param pQuotient Receives the quotient.
 * @param pRemainder Receives the remainder.
 * @ghidraAddress NTSC-U/C: 0x002ee1c8
 * @ghidraAddress PAL: 0x002f8138
 */
void RtLargeIntDivide(const RtLargeInt *pDividend,
                      const RtLargeInt *pDivisor,
                      RtLargeInt *pQuotient,
                      RtLargeInt *pRemainder);

/**
 * Generate a random prime.
 *
 * @param pPrime Receives the prime.
 * @param uBits The bits of the prime.
 * @ghidraAddress NTSC-U/C: 0x002eeddc
 * @ghidraAddress PAL: 0x002f8d4c
 */
void RtLargeIntRandomPrime(RtLargeInt *pPrime, unsigned int uBits);

/**
 * Raise an integer to a power modulo another.
 *
 * @param pResult Receives the result.
 * @param pBase The base.
 * @param pExponent The exponent.
 * @param pModulus The modulus.
 * @ghidraAddress NTSC-U/C: 0x002ef0c0
 * @ghidraAddress PAL: 0x002f9030
 */
void RtLargeIntModPower(RtLargeInt *pResult,
                        const RtLargeInt *pBase,
                        const RtLargeInt *pExponent,
                        const RtLargeInt *pModulus);

#ifdef __cplusplus
}
#endif

#endif

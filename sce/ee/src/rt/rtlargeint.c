#include "rt/rtlargeint.h"

#include <assert.h>

#include "rt/rtrandom.h"

enum {
    kLimbBits = 32,
    kLimbIndexShift = 5,
    kBitIndexMask = kLimbBits - 1,
    kRandomSeed = 0x2eb3ae77,
    // Candidates start at a multiple of this product of 2, 3, 5, and 7 and step through the
    // residues coprime to it.
    kWheelModulus = 210,
    // The number of candidates tried between two fresh random starting points.
    kAttemptsPerStart = 128,
    kMillerRabinBase = 2,
    kEulerBase = 3,
};

// NTSC-U/C: 0x003b7778, PAL: 0x00436638, the product of the primes from 11 to 743.
static const RtLargeInt g_abLintPrimesProductLow = {{
    0x88c3bed7, 0xe47d1b3c, 0x353dcb35, 0x9b1b9861, 0x5ebd0df7, 0x6f9b0a82, 0xed2c998e, 0x79c9e735,
    0x315fe9ff, 0x70097045, 0x27a3f685, 0xb5e99d33, 0xfdfe0e98, 0x809a8e9e, 0xff0158da, 0xd614c822,
    0x3527c543, 0x60726a1c, 0xc3debb81, 0x8077320a, 0xb9c6c2be, 0x462cd520, 0xcd6e7229, 0x281ce1b6,
    0x9b05f7b7, 0x04828ae7, 0x629cd15b, 0x23c1757d, 0x91db7bf6, 0xbd53fab4, 0xee78059f, 0x13b0e67e,
}};

// NTSC-U/C: 0x003b77f8, PAL: 0x004366b8, the product of the primes from 751 to 1471.
static const RtLargeInt g_abLintPrimesProductHigh = {{
    0xec3646b1, 0x7d5c4d5e, 0x225f89e7, 0x350321f1, 0x89e51646, 0x61d00e08, 0x8eec49f0, 0xa207a034,
    0x6a49487a, 0x0b95af06, 0x694e190e, 0x5a59a74f, 0xb8abd6f4, 0x774152da, 0x8e97d45e, 0x4fbbd502,
    0x4894986d, 0x33a712f8, 0xe1075a0a, 0xb023ace5, 0x82e94029, 0xd2aabfa0, 0xe6104d74, 0x34cfb7fb,
    0xdcc52a5a, 0xf673b4bc, 0xd046689e, 0x954508a4, 0xc180653e, 0x9b663907, 0x5b0c5669, 0x0481b0a8,
}};

// NTSC-U/C: 0x003b7878, PAL: 0x00436738, the steps between the residues modulo kWheelModulus that
// are coprime to it. An odd entry also restarts the walk at the first entry.
static const unsigned int g_dwLintWheelSteps[] = {
    1, 10, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2, 6, 4, 6, 8, 4,  2, 4,
    2, 4,  8, 6, 4, 6, 2, 4, 6, 2, 6, 6, 4, 2, 4, 6, 2, 6, 4, 2, 4, 2, 10, 1,
};

// NTSC-U/C: 0x003b79c0, PAL: 0x00436880
static char g_bLintRandomSeeded;

void RtLargeIntSet(RtLargeInt *value, unsigned int word) {
    const RtLargeInt result = {{word}};

    *value = result;
}

// NTSC-U/C: 0x002edcd4, PAL: 0x002f7c44
static int RtLargeIntIsEven(const RtLargeInt *value) {
    return (value->auWords[0] ^ 1) & 1;
}

int RtLargeIntIsZero(const RtLargeInt *value) {
    int i;

    for (i = 0; i < RT_LARGE_INT_WORDS; ++i) {
        if (value->auWords[i] != 0) {
            return 0;
        }
    }
    return 1;
}

int RtLargeIntIsOne(const RtLargeInt *value) {
    int i;

    if (value->auWords[0] != 1) {
        return 0;
    }
    for (i = 1; i < RT_LARGE_INT_WORDS; ++i) {
        if (value->auWords[i] != 0) {
            return 0;
        }
    }
    return 1;
}

// NTSC-U/C: 0x002edd64, PAL: 0x002f7cd4
static int RtLargeIntIsEqual(const RtLargeInt *left, const RtLargeInt *right) {
    int i;

    for (i = 0; i < RT_LARGE_INT_WORDS; ++i) {
        if (left->auWords[i] != right->auWords[i]) {
            return 0;
        }
    }
    return 1;
}

// NTSC-U/C: 0x002edda4, PAL: 0x002f7d14
static int RtLargeIntTestBit(const RtLargeInt *value, unsigned int bit) {
    return (value->auWords[bit >> kLimbIndexShift] & (1u << (bit & kBitIndexMask))) != 0;
}

// NTSC-U/C: 0x002eddc8, PAL: 0x002f7d38
static void RtLargeIntEnableBit(RtLargeInt *value, unsigned int bit) {
    value->auWords[bit >> kLimbIndexShift] |= 1u << (bit & kBitIndexMask);
}

// NTSC-U/C: 0x002eddec, PAL: 0x002f7d5c
static void RtLargeIntAddWord(RtLargeInt *value, unsigned int word) {
    unsigned int i;

    value->auWords[0] += word;
    if (value->auWords[0] >= word) {
        return;
    }
    for (i = 1; i < RT_LARGE_INT_WORDS; ++i) {
        if (++value->auWords[i] != 0) {
            return;
        }
    }
}

// NTSC-U/C: 0x002ede44, PAL: 0x002f7db4
static void RtLargeIntSubtractWord(RtLargeInt *value, unsigned int word) {
    unsigned int i;

    if (value->auWords[0] >= word) {
        value->auWords[0] -= word;
        return;
    }
    value->auWords[0] -= word;
    for (i = 1; i < RT_LARGE_INT_WORDS; ++i) {
        if (value->auWords[i]-- != 0) {
            return;
        }
    }
}

RtLargeInt RtLargeIntDifference(const RtLargeInt *minuend, const RtLargeInt *subtrahend) {
    RtLargeInt result = *minuend;

    RtLargeIntSubtract(&result, subtrahend);
    return result;
}

// NTSC-U/C: 0x002ee04c, PAL: 0x002f7fbc
static RtLargeInt RtLargeIntRemainder(const RtLargeInt *value, const RtLargeInt *modulus) {
    RtLargeInt result = *value;

    RtLargeIntModulo(&result, modulus);
    return result;
}

void RtLargeIntDivide(const RtLargeInt *dividend,
                      const RtLargeInt *divisor,
                      RtLargeInt *quotient,
                      RtLargeInt *remainder) {
    RtLargeInt shifted = *divisor;
    unsigned int remainderBits;
    unsigned int shiftedBits;
    unsigned int divisorBits;

    *remainder = *dividend;
    remainderBits = RtLargeIntBitLength(remainder);
    shiftedBits = RtLargeIntBitLength(&shifted);
    divisorBits = shiftedBits;
    RtLargeIntSet(quotient, 0);
    if (shiftedBits < remainderBits) {
        RtLargeIntShiftLeft(&shifted, remainderBits - shiftedBits);
        shiftedBits = remainderBits;
    }
    while (remainderBits >= divisorBits) {
        if (remainderBits < shiftedBits) {
            RtLargeIntShiftRight(&shifted, shiftedBits - remainderBits);
            shiftedBits = remainderBits;
        }
        if (RtLargeIntIsGreaterOrEqual(remainder, &shifted)) {
            RtLargeIntSubtract(remainder, &shifted);
            remainderBits = RtLargeIntBitLength(remainder);
            RtLargeIntEnableBit(quotient, shiftedBits - divisorBits);
        } else {
            if (divisorBits >= shiftedBits) {
                break;
            }
            RtLargeIntShiftRight(&shifted, 1);
            --shiftedBits;
        }
    }
}

// NTSC-U/C: 0x002ee418, PAL: 0x002f8388
static RtLargeInt RtLargeIntGcd(const RtLargeInt *left, const RtLargeInt *right) {
    RtLargeInt a = *left;
    RtLargeInt b = *right;

    for (;;) {
        RtLargeIntModulo(&a, &b);
        if (RtLargeIntIsZero(&a)) {
            return b;
        }
        RtLargeIntModulo(&b, &a);
        if (RtLargeIntIsZero(&b)) {
            return a;
        }
    }
}

// NTSC-U/C: 0x002ee72c, PAL: 0x002f869c
static RtLargeInt RtLargeIntSquareAndMultiply(const RtLargeInt *base,
                                              const RtLargeInt *exponent,
                                              const RtLargeInt *modulus) {
    unsigned int exponentBits = RtLargeIntBitLength(exponent);
    RtLargeInt power = RtLargeIntRemainder(base, modulus);
    RtLargeInt result;
    unsigned int mask;
    unsigned int limb;
    unsigned int i;

    if (RtLargeIntIsZero(modulus)) {
        assert(0);
        return *modulus;
    }
    RtLargeIntSet(&result, 1);
    limb = 0;
    mask = 1;
    for (i = 0; i < exponentBits; ++i) {
        if (i != 0) {
            RtLargeIntMultiplyModulo(&power, &power, &power, modulus);
            mask <<= 1;
            if (mask == 0) {
                mask = 1;
                ++limb;
            }
        }
        if ((mask & exponent->auWords[limb]) != 0) {
            RtLargeIntMultiplyModulo(&result, &result, &power, modulus);
        }
    }
    return result;
}

// NTSC-U/C: 0x002ee9b4, PAL: 0x002f8924
static int RtLargeIntMillerRabinTest(const RtLargeInt *candidate, const RtLargeInt *base) {
    RtLargeInt candidateMinusOne = *candidate;
    RtLargeInt odd;
    RtLargeInt power;
    unsigned int twos;
    unsigned int i;

    RtLargeIntSubtractWord(&candidateMinusOne, 1);
    odd = candidateMinusOne;
    twos = 0;
    while (!RtLargeIntTestBit(&odd, twos)) {
        ++twos;
    }
    if (twos != 0) {
        RtLargeIntShiftRight(&odd, twos);
    }
    power = RtLargeIntModPower(base, &odd, candidate);
    if (RtLargeIntIsOne(&power)) {
        return 1;
    }
    if (RtLargeIntIsEqual(&power, &candidateMinusOne)) {
        return 1;
    }
    for (i = 1; i < twos; ++i) {
        RtLargeIntMultiplyModulo(&power, &power, &power, candidate);
        if (RtLargeIntIsEqual(&power, &candidateMinusOne)) {
            return 1;
        }
        if (RtLargeIntIsOne(&power)) {
            return 0;
        }
    }
    return 0;
}

// NTSC-U/C: 0x002eebac, PAL: 0x002f8b1c
static int RtLargeIntEulerTest(const RtLargeInt *candidate, const RtLargeInt *base) {
    RtLargeInt power;
    RtLargeInt half = *candidate;

    RtLargeIntSubtractWord(&half, 1);
    if (!RtLargeIntIsEven(&half)) {
        return 0;
    }
    RtLargeIntShiftRight(&half, 1);
    power = RtLargeIntModPower(base, &half, candidate);
    if (RtLargeIntIsOne(&power)) {
        return 0;
    }
    RtLargeIntMultiplyModulo(&power, &power, &power, candidate);
    if (RtLargeIntIsOne(&power)) {
        return 1;
    }
    return 0;
}

// NTSC-U/C: 0x002eecfc, PAL: 0x002f8c6c
static void RtLargeIntFillRandomBits(RtLargeInt *value, unsigned int nBits) {
    unsigned int i;

    if (nBits > RT_LARGE_INT_BITS) {
        nBits = RT_LARGE_INT_BITS;
    }
    i = 0;
    if (!g_bLintRandomSeeded) {
        g_bLintRandomSeeded = 1;
        RtSeedRandomFromEntropy(kRandomSeed);
    }
    while (nBits > kLimbBits) {
        nBits -= kLimbBits;
        value->auWords[i] ^= RtRandom(); // Yes, the binary mixes in the previous words.
        ++i;
    }
    value->auWords[i] =
        (RtRandom() | (1u << kBitIndexMask)) >> ((kLimbBits - nBits) & kBitIndexMask);
    for (++i; i < RT_LARGE_INT_WORDS; ++i) {
        value->auWords[i] = 0;
    }
}

RtLargeInt RtLargeIntRandomPrime(unsigned int nBits) {
    RtLargeInt primeCandidate;
    RtLargeInt scratch;
    unsigned int attempts = 0;
    unsigned int step;
    int isPrime = 1;

    do {
        // Yes, the binary mixes random words into the uninitialised first candidate.
        RtLargeIntFillRandomBits(&primeCandidate, nBits);
        primeCandidate.auWords[0] |= 1;
        assert(nBits == RtLargeIntBitLength(&primeCandidate));
        RtLargeIntEnableBit(&primeCandidate, nBits - 2);
        RtLargeIntSet(&scratch, kWheelModulus);
        scratch = RtLargeIntRemainder(&primeCandidate, &scratch);
        RtLargeIntAddWord(&primeCandidate, kWheelModulus - scratch.auWords[0]);
        step = 0;
        while ((++attempts % kAttemptsPerStart) != 0) {
            RtLargeIntAddWord(&primeCandidate, g_dwLintWheelSteps[step++]);
            isPrime = 0;
            scratch = RtLargeIntGcd(&primeCandidate, &g_abLintPrimesProductLow);
            if (RtLargeIntIsOne(&scratch)) {
                scratch = RtLargeIntGcd(&primeCandidate, &g_abLintPrimesProductHigh);
                isPrime = RtLargeIntIsOne(&scratch) != 0;
            }
            if (isPrime) {
                RtLargeIntSet(&scratch, kMillerRabinBase);
                isPrime = RtLargeIntMillerRabinTest(&primeCandidate, &scratch);
                if (isPrime) {
                    RtLargeIntSet(&scratch, kEulerBase);
                    isPrime = RtLargeIntEulerTest(&primeCandidate, &scratch);
                    if (isPrime) {
                        return primeCandidate;
                    }
                }
            }
            if ((g_dwLintWheelSteps[step] & 1) != 0) {
                RtLargeIntAddWord(&primeCandidate, g_dwLintWheelSteps[step]);
                step = 0;
            }
        }
    } while (!isPrime);
    return primeCandidate;
}

// NTSC-U/C: 0x002ef070, PAL: 0x002f8fe0
static void RtLargeIntTruncate(RtLargeInt *value, unsigned int nBits) {
    unsigned int i = nBits >> kLimbIndexShift;

    // Yes, the binary touches the word past the end when nBits is RT_LARGE_INT_BITS.
    value->auWords[i] &= (1u << (nBits & kBitIndexMask)) - 1;
    for (++i; i < RT_LARGE_INT_WORDS; ++i) {
        value->auWords[i] = 0;
    }
}

// One Montgomery reduction of a full product, which RtLargeIntModPower() repeats inline.
static inline void RtLargeIntMontgomeryReduce(RtLargeInt *value,
                                              RtLargeInt *product,
                                              const RtLargeInt *inverse,
                                              const RtLargeInt *modulus,
                                              unsigned int nBits) {
    *value = *product;
    RtLargeIntTruncate(value, nBits);
    RtLargeIntMultiply(value, value, inverse);
    RtLargeIntTruncate(value, nBits);
    RtLargeIntMultiply(value, value, modulus);
    if (RtLargeIntTestBit(value, nBits - 1) || RtLargeIntTestBit(product, nBits - 1)) {
        // Yes, the binary guesses the carry of the low halves from their top bits.
        RtLargeIntShiftRight(value, nBits);
        RtLargeIntShiftRight(product, nBits);
        RtLargeIntAddBits(value, product, nBits);
        RtLargeIntAddWord(value, 1);
    } else {
        RtLargeIntShiftRight(value, nBits);
        RtLargeIntShiftRight(product, nBits);
        RtLargeIntAddBits(value, product, nBits);
    }
    if (RtLargeIntIsGreaterOrEqual(value, modulus)) {
        RtLargeIntSubtractBits(value, modulus, nBits);
    }
}

RtLargeInt
RtLargeIntModPower(const RtLargeInt *base, const RtLargeInt *exponent, const RtLargeInt *modulus) {
    RtLargeInt product;
    RtLargeInt power;
    RtLargeInt inverse;
    RtLargeInt result;
    unsigned int modulusBits = RtLargeIntBitLength(modulus);
    unsigned int exponentBits = RtLargeIntBitLength(exponent);
    unsigned int pending;
    unsigned int shifted;
    unsigned int mask;
    unsigned int limb;
    unsigned int i;

    if (RtLargeIntIsZero(modulus)) {
        assert(0);
        return *modulus;
    }
    if (RtLargeIntIsEven(modulus)) {
        assert(0);
        return RtLargeIntSquareAndMultiply(base, exponent, modulus);
    }

    // Build the inverse of the modulus modulo the Montgomery radix bit by bit.
    pending = 0;
    RtLargeIntSet(&inverse, 0);
    RtLargeIntSet(&result, 1);
    shifted = 0;
    while (shifted + pending < modulusBits) {
        if (RtLargeIntTestBit(&result, pending)) {
            if (pending != 0) {
                RtLargeIntShiftRight(&result, pending);
                shifted += pending;
                pending = 0;
            }
            RtLargeIntEnableBit(&inverse, shifted);
            RtLargeIntAddBits(&result, modulus, modulusBits - shifted);
        } else {
            ++pending;
        }
    }

    power = *base;
    RtLargeIntShiftLeft(&power, modulusBits);
    RtLargeIntModulo(&power, modulus);
    RtLargeIntSet(&result, 1);
    RtLargeIntShiftLeft(&result, modulusBits);
    RtLargeIntModulo(&result, modulus);
    limb = 0;
    mask = 1;
    for (i = 0; i < exponentBits; ++i) {
        if (i != 0) {
            RtLargeIntMultiply(&product, &power, &power);
            RtLargeIntMontgomeryReduce(&power, &product, &inverse, modulus, modulusBits);
            mask <<= 1;
            if (mask == 0) {
                mask = 1;
                ++limb;
            }
        }
        if ((mask & exponent->auWords[limb]) != 0) {
            RtLargeIntMultiply(&product, &result, &power);
            RtLargeIntMontgomeryReduce(&result, &product, &inverse, modulus, modulusBits);
        }
    }

    product = result;
    RtLargeIntTruncate(&result, modulusBits);
    RtLargeIntMultiply(&result, &result, &inverse);
    RtLargeIntTruncate(&result, modulusBits);
    RtLargeIntMultiply(&result, &result, modulus);
    RtLargeIntAdd(&result, &product);
    RtLargeIntShiftRight(&result, modulusBits);
    if (RtLargeIntIsGreaterOrEqual(&result, modulus)) {
        RtLargeIntSubtract(&result, modulus);
    }
    return result;
}

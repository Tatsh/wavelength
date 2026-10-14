#include <assert.h>

#include <librtbigint.h>

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

// NTSC-U/C: 0x003b7778, the product of the primes from 11 to 743.
static const rtLargeInt g_abLintPrimesProductLow = {{
    0x88c3bed7, 0xe47d1b3c, 0x353dcb35, 0x9b1b9861, 0x5ebd0df7, 0x6f9b0a82, 0xed2c998e, 0x79c9e735,
    0x315fe9ff, 0x70097045, 0x27a3f685, 0xb5e99d33, 0xfdfe0e98, 0x809a8e9e, 0xff0158da, 0xd614c822,
    0x3527c543, 0x60726a1c, 0xc3debb81, 0x8077320a, 0xb9c6c2be, 0x462cd520, 0xcd6e7229, 0x281ce1b6,
    0x9b05f7b7, 0x04828ae7, 0x629cd15b, 0x23c1757d, 0x91db7bf6, 0xbd53fab4, 0xee78059f, 0x13b0e67e,
}};

// NTSC-U/C: 0x003b77f8, the product of the primes from 751 to 1471.
static const rtLargeInt g_abLintPrimesProductHigh = {{
    0xec3646b1, 0x7d5c4d5e, 0x225f89e7, 0x350321f1, 0x89e51646, 0x61d00e08, 0x8eec49f0, 0xa207a034,
    0x6a49487a, 0x0b95af06, 0x694e190e, 0x5a59a74f, 0xb8abd6f4, 0x774152da, 0x8e97d45e, 0x4fbbd502,
    0x4894986d, 0x33a712f8, 0xe1075a0a, 0xb023ace5, 0x82e94029, 0xd2aabfa0, 0xe6104d74, 0x34cfb7fb,
    0xdcc52a5a, 0xf673b4bc, 0xd046689e, 0x954508a4, 0xc180653e, 0x9b663907, 0x5b0c5669, 0x0481b0a8,
}};

// NTSC-U/C: 0x003b7878, the steps between the residues modulo kWheelModulus that are coprime to
// it. An odd entry also restarts the walk at the first entry.
static const unsigned int g_dwLintWheelSteps[] = {
    1, 10, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2, 6, 4, 6, 8, 4,  2, 4,
    2, 4,  8, 6, 4, 6, 2, 4, 6, 2, 6, 6, 4, 2, 4, 6, 2, 6, 4, 2, 4, 2, 10, 1,
};

// NTSC-U/C: 0x003b79c0
static char g_bLintRandomSeeded;

void lintSet(rtLargeInt *value, unsigned int word) {
    const rtLargeInt result = {{word}};

    *value = result;
}

// NTSC-U/C: 0x002edcd4, PAL: 0x002f7c44
static int lintIsEven(const rtLargeInt *value) {
    return (value->limbs[0] ^ 1) & 1;
}

int lintIsZero(const rtLargeInt *value) {
    int i;

    for (i = 0; i < RT_LARGE_INT_LIMB_COUNT; ++i) {
        if (value->limbs[i] != 0) {
            return 0;
        }
    }
    return 1;
}

int lintIsOne(const rtLargeInt *value) {
    int i;

    if (value->limbs[0] != 1) {
        return 0;
    }
    for (i = 1; i < RT_LARGE_INT_LIMB_COUNT; ++i) {
        if (value->limbs[i] != 0) {
            return 0;
        }
    }
    return 1;
}

// NTSC-U/C: 0x002edd64, PAL: 0x002f7cd4
static int lintIsEqual(const rtLargeInt *left, const rtLargeInt *right) {
    int i;

    for (i = 0; i < RT_LARGE_INT_LIMB_COUNT; ++i) {
        if (left->limbs[i] != right->limbs[i]) {
            return 0;
        }
    }
    return 1;
}

// NTSC-U/C: 0x002edda4, PAL: 0x002f7d14
static int lintTestBit(const rtLargeInt *value, unsigned int bit) {
    return (value->limbs[bit >> kLimbIndexShift] & (1u << (bit & kBitIndexMask))) != 0;
}

// NTSC-U/C: 0x002eddc8, PAL: 0x002f7d38
static void lintSetBit(rtLargeInt *value, unsigned int bit) {
    value->limbs[bit >> kLimbIndexShift] |= 1u << (bit & kBitIndexMask);
}

// NTSC-U/C: 0x002eddec, PAL: 0x002f7d5c
static void lintAddWord(rtLargeInt *value, unsigned int word) {
    unsigned int i;

    value->limbs[0] += word;
    if (value->limbs[0] >= word) {
        return;
    }
    for (i = 1; i < RT_LARGE_INT_LIMB_COUNT; ++i) {
        if (++value->limbs[i] != 0) {
            return;
        }
    }
}

// NTSC-U/C: 0x002ede44, PAL: 0x002f7db4
static void lintSubtractWord(rtLargeInt *value, unsigned int word) {
    unsigned int i;

    if (value->limbs[0] >= word) {
        value->limbs[0] -= word;
        return;
    }
    value->limbs[0] -= word;
    for (i = 1; i < RT_LARGE_INT_LIMB_COUNT; ++i) {
        if (value->limbs[i]-- != 0) {
            return;
        }
    }
}

rtLargeInt lintDifference(const rtLargeInt *minuend, const rtLargeInt *subtrahend) {
    rtLargeInt result = *minuend;

    lintSubtract(&result, subtrahend);
    return result;
}

// NTSC-U/C: 0x002ee04c, PAL: 0x002f7fbc
static rtLargeInt lintRemainder(const rtLargeInt *value, const rtLargeInt *modulus) {
    rtLargeInt result = *value;

    lintMod(&result, modulus);
    return result;
}

void lintDivide(const rtLargeInt *dividend,
                const rtLargeInt *divisor,
                rtLargeInt *quotient,
                rtLargeInt *remainder) {
    rtLargeInt shifted = *divisor;
    unsigned int remainderBits;
    unsigned int shiftedBits;
    unsigned int divisorBits;

    *remainder = *dividend;
    remainderBits = lintGetBits(remainder);
    shiftedBits = lintGetBits(&shifted);
    divisorBits = shiftedBits;
    lintSet(quotient, 0);
    if (shiftedBits < remainderBits) {
        lintShiftLeft(&shifted, remainderBits - shiftedBits);
        shiftedBits = remainderBits;
    }
    while (remainderBits >= divisorBits) {
        if (remainderBits < shiftedBits) {
            lintShiftRight(&shifted, shiftedBits - remainderBits);
            shiftedBits = remainderBits;
        }
        if (lintIsGreaterOrEqual(remainder, &shifted)) {
            lintSubtract(remainder, &shifted);
            remainderBits = lintGetBits(remainder);
            lintSetBit(quotient, shiftedBits - divisorBits);
        } else {
            if (divisorBits >= shiftedBits) {
                break;
            }
            lintShiftRight(&shifted, 1);
            --shiftedBits;
        }
    }
}

// NTSC-U/C: 0x002ee418, PAL: 0x002f8388
static rtLargeInt lintGcd(const rtLargeInt *left, const rtLargeInt *right) {
    rtLargeInt a = *left;
    rtLargeInt b = *right;

    for (;;) {
        lintMod(&a, &b);
        if (lintIsZero(&a)) {
            return b;
        }
        lintMod(&b, &a);
        if (lintIsZero(&b)) {
            return a;
        }
    }
}

// NTSC-U/C: 0x002ee72c, PAL: 0x002f869c
static rtLargeInt
lintModExp(const rtLargeInt *base, const rtLargeInt *exponent, const rtLargeInt *modulus) {
    unsigned int exponentBits = lintGetBits(exponent);
    rtLargeInt power = lintRemainder(base, modulus);
    rtLargeInt result;
    unsigned int mask;
    unsigned int limb;
    unsigned int i;

    if (lintIsZero(modulus)) {
        assert(0);
        return *modulus;
    }
    lintSet(&result, 1);
    limb = 0;
    mask = 1;
    for (i = 0; i < exponentBits; ++i) {
        if (i != 0) {
            lintMultiplyMod(&power, &power, &power, modulus);
            mask <<= 1;
            if (mask == 0) {
                mask = 1;
                ++limb;
            }
        }
        if ((mask & exponent->limbs[limb]) != 0) {
            lintMultiplyMod(&result, &result, &power, modulus);
        }
    }
    return result;
}

// NTSC-U/C: 0x002ee9b4, PAL: 0x002f8924
static int lintMillerRabinTest(const rtLargeInt *candidate, const rtLargeInt *base) {
    rtLargeInt candidateMinusOne = *candidate;
    rtLargeInt odd;
    rtLargeInt power;
    unsigned int twos;
    unsigned int i;

    lintSubtractWord(&candidateMinusOne, 1);
    odd = candidateMinusOne;
    twos = 0;
    while (!lintTestBit(&odd, twos)) {
        ++twos;
    }
    if (twos != 0) {
        lintShiftRight(&odd, twos);
    }
    power = lintMontgomeryModExp(base, &odd, candidate);
    if (lintIsOne(&power)) {
        return 1;
    }
    if (lintIsEqual(&power, &candidateMinusOne)) {
        return 1;
    }
    for (i = 1; i < twos; ++i) {
        lintMultiplyMod(&power, &power, &power, candidate);
        if (lintIsEqual(&power, &candidateMinusOne)) {
            return 1;
        }
        if (lintIsOne(&power)) {
            return 0;
        }
    }
    return 0;
}

// NTSC-U/C: 0x002eebac, PAL: 0x002f8b1c
static int lintEulerTest(const rtLargeInt *candidate, const rtLargeInt *base) {
    rtLargeInt power;
    rtLargeInt half = *candidate;

    lintSubtractWord(&half, 1);
    if (!lintIsEven(&half)) {
        return 0;
    }
    lintShiftRight(&half, 1);
    power = lintMontgomeryModExp(base, &half, candidate);
    if (lintIsOne(&power)) {
        return 0;
    }
    lintMultiplyMod(&power, &power, &power, candidate);
    if (lintIsOne(&power)) {
        return 1;
    }
    return 0;
}

// NTSC-U/C: 0x002eecfc, PAL: 0x002f8c6c
static void lintRandom(rtLargeInt *value, unsigned int nBits) {
    unsigned int i;

    if (nBits > RT_LARGE_INT_BITS) {
        nBits = RT_LARGE_INT_BITS;
    }
    i = 0;
    if (!g_bLintRandomSeeded) {
        g_bLintRandomSeeded = 1;
        rtDmeSeedRandom(kRandomSeed);
    }
    while (nBits > kLimbBits) {
        nBits -= kLimbBits;
        value->limbs[i] ^= rtDmeRandom(); // Yes, the binary mixes in the previous words.
        ++i;
    }
    value->limbs[i] =
        (rtDmeRandom() | (1u << kBitIndexMask)) >> ((kLimbBits - nBits) & kBitIndexMask);
    for (++i; i < RT_LARGE_INT_LIMB_COUNT; ++i) {
        value->limbs[i] = 0;
    }
}

rtLargeInt lintGeneratePrime(unsigned int nBits) {
    rtLargeInt primeCandidate;
    rtLargeInt scratch;
    unsigned int attempts = 0;
    unsigned int step;
    int isPrime = 1;

    do {
        // Yes, the binary mixes random words into the uninitialised first candidate.
        lintRandom(&primeCandidate, nBits);
        primeCandidate.limbs[0] |= 1;
        assert(nBits == lintGetBits(&primeCandidate));
        lintSetBit(&primeCandidate, nBits - 2);
        lintSet(&scratch, kWheelModulus);
        scratch = lintRemainder(&primeCandidate, &scratch);
        lintAddWord(&primeCandidate, kWheelModulus - scratch.limbs[0]);
        step = 0;
        while ((++attempts % kAttemptsPerStart) != 0) {
            lintAddWord(&primeCandidate, g_dwLintWheelSteps[step++]);
            isPrime = 0;
            scratch = lintGcd(&primeCandidate, &g_abLintPrimesProductLow);
            if (lintIsOne(&scratch)) {
                scratch = lintGcd(&primeCandidate, &g_abLintPrimesProductHigh);
                isPrime = lintIsOne(&scratch) != 0;
            }
            if (isPrime) {
                lintSet(&scratch, kMillerRabinBase);
                isPrime = lintMillerRabinTest(&primeCandidate, &scratch);
                if (isPrime) {
                    lintSet(&scratch, kEulerBase);
                    isPrime = lintEulerTest(&primeCandidate, &scratch);
                    if (isPrime) {
                        return primeCandidate;
                    }
                }
            }
            if ((g_dwLintWheelSteps[step] & 1) != 0) {
                lintAddWord(&primeCandidate, g_dwLintWheelSteps[step]);
                step = 0;
            }
        }
    } while (!isPrime);
    return primeCandidate;
}

// NTSC-U/C: 0x002ef070, PAL: 0x002f8fe0
static void lintTruncate(rtLargeInt *value, unsigned int nBits) {
    unsigned int i = nBits >> kLimbIndexShift;

    // Yes, the binary touches the word past the end when nBits is RT_LARGE_INT_BITS.
    value->limbs[i] &= (1u << (nBits & kBitIndexMask)) - 1;
    for (++i; i < RT_LARGE_INT_LIMB_COUNT; ++i) {
        value->limbs[i] = 0;
    }
}

// One Montgomery reduction of a full product, which lintMontgomeryModExp() repeats inline.
static inline void lintMontgomeryReduce(rtLargeInt *value,
                                        rtLargeInt *product,
                                        const rtLargeInt *inverse,
                                        const rtLargeInt *modulus,
                                        unsigned int nBits) {
    *value = *product;
    lintTruncate(value, nBits);
    lintMultiply(value, value, inverse);
    lintTruncate(value, nBits);
    lintMultiply(value, value, modulus);
    if (lintTestBit(value, nBits - 1) || lintTestBit(product, nBits - 1)) {
        // Yes, the binary guesses the carry of the low halves from their top bits.
        lintShiftRight(value, nBits);
        lintShiftRight(product, nBits);
        lintAddBits(value, product, nBits);
        lintAddWord(value, 1);
    } else {
        lintShiftRight(value, nBits);
        lintShiftRight(product, nBits);
        lintAddBits(value, product, nBits);
    }
    if (lintIsGreaterOrEqual(value, modulus)) {
        lintSubtractBits(value, modulus, nBits);
    }
}

rtLargeInt lintMontgomeryModExp(const rtLargeInt *base,
                                const rtLargeInt *exponent,
                                const rtLargeInt *modulus) {
    rtLargeInt product;
    rtLargeInt power;
    rtLargeInt inverse;
    rtLargeInt result;
    unsigned int modulusBits = lintGetBits(modulus);
    unsigned int exponentBits = lintGetBits(exponent);
    unsigned int pending;
    unsigned int shifted;
    unsigned int mask;
    unsigned int limb;
    unsigned int i;

    if (lintIsZero(modulus)) {
        assert(0);
        return *modulus;
    }
    if (lintIsEven(modulus)) {
        assert(0);
        return lintModExp(base, exponent, modulus);
    }

    // Build the inverse of the modulus modulo the Montgomery radix bit by bit.
    pending = 0;
    lintSet(&inverse, 0);
    lintSet(&result, 1);
    shifted = 0;
    while (shifted + pending < modulusBits) {
        if (lintTestBit(&result, pending)) {
            if (pending != 0) {
                lintShiftRight(&result, pending);
                shifted += pending;
                pending = 0;
            }
            lintSetBit(&inverse, shifted);
            lintAddBits(&result, modulus, modulusBits - shifted);
        } else {
            ++pending;
        }
    }

    power = *base;
    lintShiftLeft(&power, modulusBits);
    lintMod(&power, modulus);
    lintSet(&result, 1);
    lintShiftLeft(&result, modulusBits);
    lintMod(&result, modulus);
    limb = 0;
    mask = 1;
    for (i = 0; i < exponentBits; ++i) {
        if (i != 0) {
            lintMultiply(&product, &power, &power);
            lintMontgomeryReduce(&power, &product, &inverse, modulus, modulusBits);
            mask <<= 1;
            if (mask == 0) {
                mask = 1;
                ++limb;
            }
        }
        if ((mask & exponent->limbs[limb]) != 0) {
            lintMultiply(&product, &result, &power);
            lintMontgomeryReduce(&result, &product, &inverse, modulus, modulusBits);
        }
    }

    product = result;
    lintTruncate(&result, modulusBits);
    lintMultiply(&result, &result, &inverse);
    lintTruncate(&result, modulusBits);
    lintMultiply(&result, &result, modulus);
    lintAdd(&result, &product);
    lintShiftRight(&result, modulusBits);
    if (lintIsGreaterOrEqual(&result, modulus)) {
        lintSubtract(&result, modulus);
    }
    return result;
}

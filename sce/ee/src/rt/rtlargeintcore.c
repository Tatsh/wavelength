#include "rt/rtlargeint.h"

enum {
    kWordBits = 32,
    kWordShift = 5,
    kBitMask = 31,
};

// Shift the low words of an integer right in place. The words from uWords up are left as they are.
static inline void
RtLargeIntShiftWordsRight(unsigned int *puWords, unsigned int uBits, unsigned int uWords) {
    unsigned int uWordShift;
    unsigned int uBitShift;
    unsigned int uPrevious;
    unsigned int uCurrent;
    unsigned int i;
    unsigned int j;

    uWordShift = uBits >> kWordShift;
    uBitShift = uBits & kBitMask;
    uPrevious = puWords[uWordShift];
    j = 0;
    for (i = uWordShift + 1; i < uWords; ++i) {
        uCurrent = puWords[i];
        puWords[j++] = (unsigned int)(((unsigned long long)uPrevious >> uBitShift) |
                                      ((unsigned long long)uCurrent << (kWordBits - uBitShift)));
        uPrevious = uCurrent;
    }
    puWords[j++] = uPrevious >> uBitShift;
    for (i = 0; i < uWordShift; ++i) {
        puWords[j++] = 0;
    }
}

// Shift the low words of an integer left in place. The words from uWords up are left as they are.
static inline void
RtLargeIntShiftWordsLeft(unsigned int *puWords, unsigned int uBits, unsigned int uWords) {
    unsigned int uWordShift;
    unsigned int uBitShift;
    unsigned int uPrevious;
    unsigned int uCurrent;
    unsigned int i;
    unsigned int j;

    uWordShift = uBits >> kWordShift;
    uBitShift = uBits & kBitMask;
    i = uWords - 1 - uWordShift;
    j = uWords;
    uPrevious = puWords[i];
    while (i != 0) {
        uCurrent = puWords[--i];
        puWords[--j] = (unsigned int)(((unsigned long long)uPrevious << uBitShift) |
                                      ((unsigned long long)uCurrent >> (kWordBits - uBitShift)));
        uPrevious = uCurrent;
    }
    puWords[--j] = uPrevious << uBitShift;
    while (j != 0) {
        puWords[--j] = 0;
    }
}

// Subtract the low words of one integer from another in place, wrapping below zero.
static inline void RtLargeIntSubtractWords(unsigned int *puDifference,
                                           const unsigned int *puSubtrahend,
                                           unsigned int uWords) {
    long long llBorrow;
    unsigned int i;

    llBorrow = 0;
    for (i = 0; i < uWords; ++i) {
        llBorrow += puDifference[i];
        llBorrow -= puSubtrahend[i];
        puDifference[i] = (unsigned int)llBorrow;
        llBorrow >>= kWordBits;
    }
}

// Add the low words of one integer to another in place.
static inline void
RtLargeIntAddWords(unsigned int *puSum, const unsigned int *puAddend, unsigned int uWords) {
    unsigned long long ullCarry;
    unsigned int i;

    ullCarry = 0;
    for (i = 0; i < uWords; ++i) {
        ullCarry += puSum[i];
        ullCarry += puAddend[i];
        puSum[i] = (unsigned int)ullCarry;
        ullCarry >>= kWordBits;
    }
}

// Multiply two integers into the words of a product that starts at zero.
static inline void
RtLargeIntMultiplyInto(RtLargeInt *pProduct, const RtLargeInt *pLeft, const RtLargeInt *pRight) {
    RtLargeInt lowParts;
    RtLargeInt highParts;
    unsigned long long ullPart;
    unsigned long long ullCarry;
    unsigned int uLeftBits;
    unsigned int uRightBits;
    unsigned int uLeftWords;
    unsigned int uRightWords;
    unsigned int uProductWords;
    unsigned int i;
    unsigned int j;

    uLeftBits = RtLargeIntBitLength(pLeft);
    uLeftWords = (uLeftBits + kBitMask) >> kWordShift;
    uRightBits = RtLargeIntBitLength(pRight);
    uRightWords = (uRightBits + kBitMask) >> kWordShift;
    uProductWords = ((uLeftBits + uRightBits) >> kWordShift) + 1;
    if (uProductWords >= RT_LARGE_INT_WORDS) {
        uProductWords = RT_LARGE_INT_WORDS;
    }
    RtLargeIntSet(pProduct, 0);
    RtLargeIntSet(&lowParts, 0);
    RtLargeIntSet(&highParts, 0);
    for (i = 0; i < uLeftWords; ++i) {
        for (j = 0; j < uRightWords; ++j) {
            ullPart = (unsigned long long)pLeft->auWords[i] * pRight->auWords[j];
            lowParts.auWords[i + j] = (unsigned int)ullPart;
            highParts.auWords[i + j + 1] = (unsigned int)(ullPart >> kWordBits);
        }
        ullCarry = 0;
        for (j = 0; j < uProductWords; ++j) {
            ullCarry += pProduct->auWords[j];
            ullCarry += lowParts.auWords[j];
            ullCarry += highParts.auWords[j];
            pProduct->auWords[j] = (unsigned int)ullCarry;
            ullCarry >>= kWordBits;
        }
        lowParts.auWords[i] = 0;
        highParts.auWords[i + 1] = 0;
    }
}

unsigned int RtLargeIntBitLength(const RtLargeInt *pValue) {
    unsigned int uBits;
    unsigned int uWord;
    unsigned int uMask;
    int i;

    uBits = RT_LARGE_INT_BITS;
    for (i = RT_LARGE_INT_WORDS - 1; i >= 0; --i) {
        uWord = pValue->auWords[i];
        if (uWord == 0) {
            uBits -= kWordBits;
            continue;
        }
        for (uMask = 1U << kBitMask; uMask != 0; uMask >>= 1) {
            if (uWord >= uMask) {
                return uBits;
            }
            --uBits;
        }
    }
    return uBits;
}

int RtLargeIntIsGreater(const RtLargeInt *pLeft, const RtLargeInt *pRight) {
    int i;

    for (i = RT_LARGE_INT_WORDS - 1; i >= 0; --i) {
        if (pLeft->auWords[i] > pRight->auWords[i]) {
            return 1;
        }
        if (pLeft->auWords[i] < pRight->auWords[i]) {
            return 0;
        }
    }
    return 0;
}

int RtLargeIntIsLess(const RtLargeInt *pLeft, const RtLargeInt *pRight) {
    int i;

    for (i = RT_LARGE_INT_WORDS - 1; i >= 0; --i) {
        if (pLeft->auWords[i] < pRight->auWords[i]) {
            return 1;
        }
        if (pLeft->auWords[i] > pRight->auWords[i]) {
            return 0;
        }
    }
    return 0;
}

int RtLargeIntIsGreaterOrEqual(const RtLargeInt *pLeft, const RtLargeInt *pRight) {
    int i;

    for (i = RT_LARGE_INT_WORDS - 1; i >= 0; --i) {
        if (pLeft->auWords[i] > pRight->auWords[i]) {
            return 1;
        }
        if (pLeft->auWords[i] < pRight->auWords[i]) {
            return 0;
        }
    }
    return 1;
}

void RtLargeIntShiftRight(RtLargeInt *pValue, unsigned int uBits) {
    unsigned int uWordShift;
    unsigned int uWords;

    // Shift only the words up to the highest non-zero one.
    uWordShift = uBits >> kWordShift;
    for (uWords = RT_LARGE_INT_WORDS; uWords != uWordShift; --uWords) {
        if (pValue->auWords[uWords - 1] != 0) {
            break;
        }
    }
    if (uWords == uWordShift) {
        ++uWords;
    }
    RtLargeIntShiftWordsRight(pValue->auWords, uBits, uWords);
}

void RtLargeIntShiftLeft(RtLargeInt *pValue, unsigned int uBits) {
    unsigned int uWordShift;
    unsigned int uTop;

    // Shift only the words up to one past the highest that receives a non-zero word.
    uWordShift = uBits >> kWordShift;
    for (uTop = RT_LARGE_INT_WORDS - 1;; --uTop) {
        if (pValue->auWords[uTop - uWordShift] != 0) {
            if (uTop != RT_LARGE_INT_WORDS - 1) {
                ++uTop;
            }
            break;
        }
        if (uTop == uWordShift) {
            // Every word shifted is zero, and shifting them all gives the same zero.
            uTop = RT_LARGE_INT_WORDS - 1;
            break;
        }
    }
    RtLargeIntShiftWordsLeft(pValue->auWords, uBits, uTop + 1);
}

void RtLargeIntAdd(RtLargeInt *pSum, const RtLargeInt *pAddend) {
    RtLargeIntAddWords(pSum->auWords, pAddend->auWords, RT_LARGE_INT_WORDS);
}

void RtLargeIntAddBits(RtLargeInt *pSum, const RtLargeInt *pAddend, unsigned int uBits) {
    unsigned int uWords;

    uWords = (uBits >> kWordShift) + 1;
    if (uWords >= RT_LARGE_INT_WORDS) {
        uWords = RT_LARGE_INT_WORDS;
    }
    RtLargeIntAddWords(pSum->auWords, pAddend->auWords, uWords);
}

void RtLargeIntSubtract(RtLargeInt *pDifference, const RtLargeInt *pSubtrahend) {
    RtLargeIntSubtractWords(pDifference->auWords, pSubtrahend->auWords, RT_LARGE_INT_WORDS);
}

void RtLargeIntSubtractBits(RtLargeInt *pDifference,
                            const RtLargeInt *pSubtrahend,
                            unsigned int uBits) {
    unsigned int uWords;

    uWords = (uBits >> kWordShift) + 1;
    if (uWords >= RT_LARGE_INT_WORDS) {
        uWords = RT_LARGE_INT_WORDS;
    }
    RtLargeIntSubtractWords(pDifference->auWords, pSubtrahend->auWords, uWords);
}

void RtLargeIntModulo(RtLargeInt *pValue, const RtLargeInt *pModulus) {
    RtLargeInt modulus;
    unsigned int uValueBits;
    unsigned int uModulusBits;
    unsigned int uShiftedBits;
    unsigned int uWords;
    int i;

    modulus = *pModulus;
    uValueBits = RtLargeIntBitLength(pValue);
    uModulusBits = RtLargeIntBitLength(&modulus);
    uShiftedBits = uModulusBits;
    if (uModulusBits < uValueBits) {
        RtLargeIntShiftWordsLeft(
            modulus.auWords, uValueBits - uModulusBits, (uValueBits + kBitMask) >> kWordShift);
        uShiftedBits = uValueBits;
    }
    while (uValueBits >= uModulusBits) {
        if (uValueBits < uShiftedBits) {
            // Yes, the binary shifts only the words the value needs, and a large drop in the bits
            // of the value loses the bits of the modulus above them.
            uWords = (uValueBits + kBitMask) >> kWordShift;
            if (uWords < RT_LARGE_INT_WORDS) {
                ++uWords;
            }
            RtLargeIntShiftWordsRight(modulus.auWords, uShiftedBits - uValueBits, uWords);
            uShiftedBits = uValueBits;
        }
        for (i = (uValueBits - 1) >> kWordShift; i >= 0; --i) {
            if (pValue->auWords[i] != modulus.auWords[i]) {
                break;
            }
        }
        if (i < 0 || pValue->auWords[i] > modulus.auWords[i]) {
            RtLargeIntSubtractWords(
                pValue->auWords, modulus.auWords, (uValueBits + kBitMask) >> kWordShift);
            uValueBits = RtLargeIntBitLength(pValue);
            continue;
        }
        if (uShiftedBits <= uModulusBits) {
            return;
        }
        --uShiftedBits;
        uWords = (uShiftedBits + kBitMask) >> kWordShift;
        if (uWords < RT_LARGE_INT_WORDS) {
            ++uWords;
        }
        RtLargeIntShiftWordsRight(modulus.auWords, 1, uWords);
    }
}

void RtLargeIntMultiply(RtLargeInt *pProduct, const RtLargeInt *pLeft, const RtLargeInt *pRight) {
    RtLargeInt product;

    RtLargeIntMultiplyInto(&product, pLeft, pRight);
    *pProduct = product;
}

void RtLargeIntMultiplyModulo(RtLargeInt *pProduct,
                              const RtLargeInt *pLeft,
                              const RtLargeInt *pRight,
                              const RtLargeInt *pModulus) {
    RtLargeInt product;

    RtLargeIntMultiplyInto(&product, pLeft, pRight);
    RtLargeIntModulo(&product, pModulus);
    *pProduct = product;
}

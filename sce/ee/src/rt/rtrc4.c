#include "rt/rtrc4.h"

enum {
    kRc4KeyMask = 63,
    kRc4SaltMask = 3,
    kRc4KeyStep = 3,
    kRc4SaltStep = 5,
    kRc4CipherStep = 5,
};

// Fill the permutation with the bytes in descending order.
static inline void RtRc4Reset(RtRc4 *pRc4) {
    unsigned char i;

    i = 0;
    do {
        pRc4->abState[i] = ~i;
        ++i;
    } while (i != 0);
}

// Mix a key into the permutation, stepping through it.
static inline void
RtRc4Mix(RtRc4 *pRc4, const unsigned char *pbKey, unsigned int uKeyMask, unsigned char uStep) {
    unsigned char uSwap;
    unsigned char i;
    unsigned char j;
    unsigned int uKeyIndex;

    j = 0;
    uKeyIndex = 0;
    i = 0;
    do {
        uSwap = pRc4->abState[i];
        j = j + pbKey[uKeyIndex] + uSwap;
        pRc4->abState[i] = pRc4->abState[j];
        uKeyIndex = (uKeyIndex + 1) & uKeyMask;
        i += uStep;
        pRc4->abState[j] = uSwap;
    } while (i != 0);
}

void RtRc4SetKey(RtRc4 *pRc4, const RtKey *pKey) {
    RtRc4Reset(pRc4);
    RtRc4Mix(pRc4, pKey->bytes, kRc4KeyMask, kRc4KeyStep);
    pRc4->uI = 0;
    pRc4->uJ = 0;
    pRc4->uCount = 0;
}

void RtRc4SetSaltedKey(RtRc4 *pRc4, const RtKey *pKey, unsigned int uSalt) {
    union {
        unsigned int uWord;
        unsigned char abBytes[sizeof(unsigned int)];
    } salt;

    salt.uWord = uSalt;
    RtRc4Reset(pRc4);
    RtRc4Mix(pRc4, salt.abBytes, kRc4SaltMask, kRc4SaltStep);
    RtRc4Mix(pRc4, pKey->bytes, kRc4KeyMask, kRc4KeyStep);
    pRc4->uI = 0;
    pRc4->uJ = 0;
    pRc4->uCount = 0;
}

void RtRc4Encrypt(RtRc4 *pRc4, unsigned char *pbData, unsigned int uSize) {
    unsigned char uI;
    unsigned char uJ;
    unsigned char uSwap;
    unsigned char uIndex;
    unsigned int n;

    uI = pRc4->uI;
    uJ = pRc4->uJ;
    for (n = 0; n < uSize; ++n) {
        uI += kRc4CipherStep;
        uJ += pRc4->abState[uI];
        uSwap = pRc4->abState[uI];
        pRc4->abState[uI] = pRc4->abState[uJ];
        pRc4->abState[uJ] = uSwap;
        uIndex = pRc4->abState[uI] + uSwap;
        uJ += pRc4->abState[pbData[n]];
        pbData[n] ^= pRc4->abState[uIndex];
    }
    pRc4->uI = uI;
    pRc4->uJ = uJ;
    pRc4->uCount += uSize;
}

void RtRc4Decrypt(RtRc4 *pRc4, unsigned char *pbData, unsigned int uSize) {
    unsigned char uI;
    unsigned char uJ;
    unsigned char uSwap;
    unsigned char uIndex;
    unsigned int n;

    uI = pRc4->uI;
    uJ = pRc4->uJ;
    for (n = 0; n < uSize; ++n) {
        uI += kRc4CipherStep;
        uJ += pRc4->abState[uI];
        uSwap = pRc4->abState[uI];
        pRc4->abState[uI] = pRc4->abState[uJ];
        pRc4->abState[uJ] = uSwap;
        uIndex = pRc4->abState[uI] + uSwap;
        pbData[n] ^= pRc4->abState[uIndex];
        uJ += pRc4->abState[pbData[n]];
    }
    pRc4->uI = uI;
    pRc4->uJ = uJ;
    pRc4->uCount += uSize;
}

#include "rt/rtrsa.h"

#include <assert.h>
#include <string.h>

enum {
    kRsaKeyBits = 512,
    kRsaPrimeBits = 256,
    kRsaPublicExponent = 17,
};

// NTSC-U/C: 0x003b6c80, PAL: 0x00435b40
static RtKey g_rsaPublicExponent = {.words = {kRsaPublicExponent}};

void RtRsaGenerateKeyPair(RtKey *pModulus, RtKey *pPrivateExponent) {
    RtLargeInt p;
    RtLargeInt q;
    RtLargeInt x1;
    RtLargeInt x2;
    RtLargeInt r1;
    RtLargeInt r2;
    RtLargeInt phi;
    RtLargeInt check;
    RtLargeInt e;
    RtLargeInt d;
    RtLargeInt n;

    RtLargeIntSet(&d, 0);
    q = RtLargeIntRandomPrime(kRsaPrimeBits);
    do {
        p = q;
        q = RtLargeIntRandomPrime(kRsaKeyBits / 2);
        RtLargeIntMultiply(&n, &p, &q);
        RtLargeIntSet(&phi, 1);
        RtLargeIntAdd(&phi, &n);
        RtLargeIntSubtract(&phi, &p);
        RtLargeIntSubtract(&phi, &q);
        RtLargeIntSet(&e, kRsaPublicExponent);
        RtLargeIntSet(&x1, 1);
        r1 = e;
        RtLargeIntDivide(&phi, &e, &x2, &r2);
        RtLargeIntAdd(&x2, &x1);
        r2 = RtLargeIntDifference(&e, &r2);
        for (;;) {
            if (RtLargeIntIsGreater(&r1, &r2)) {
                if (RtLargeIntIsLess(&x1, &x2)) {
                    RtLargeIntAdd(&x1, &phi);
                }
                RtLargeIntSubtract(&x1, &x2);
                RtLargeIntSubtract(&r1, &r2);
                if (RtLargeIntIsOne(&r1)) {
                    d = x1;
                    break;
                }
            } else if (RtLargeIntIsLess(&r1, &r2)) {
                if (RtLargeIntIsLess(&x2, &x1)) {
                    RtLargeIntAdd(&x2, &phi);
                }
                RtLargeIntSubtract(&x2, &x1);
                RtLargeIntSubtract(&r2, &r1);
                if (RtLargeIntIsOne(&r2)) {
                    d = x2;
                    break;
                }
            } else {
                break;
            }
        }
    } while (RtLargeIntIsZero(&d));
    RtLargeIntMultiplyModulo(&check, &d, &e, &phi);
    if (!RtLargeIntIsOne(&check)) {
        assert(0);
    }
    *pModulus = n.aHalves[0];
    *pPrivateExponent = d.aHalves[0];
}

// NTSC-U/C: 0x002bf7e4, PAL: 0x002c9754
static void
RtRsaTransformBlock(const RtKey *pModulus, const RtKey *pExponent, const RtKey *pIn, RtKey *pOut) {
    RtLargeInt exponent;
    RtLargeInt modulus;
    RtLargeInt base;
    RtLargeInt result;

    exponent.aHalves[0] = *pExponent;
    memset(&exponent.aHalves[1], 0, sizeof(RtKey));
    modulus.aHalves[0] = *pModulus;
    memset(&modulus.aHalves[1], 0, sizeof(RtKey));
    base.aHalves[0] = *pIn;
    memset(&base.aHalves[1], 0, sizeof(RtKey));
    result = RtLargeIntModPower(&base, &exponent, &modulus);
    *pOut = result.aHalves[0];
}

unsigned int
RtRsaEncryptPublic(const RtKey *pModulus, const RtKey *pIn, RtKey *pOut, unsigned int uSize) {
    return RtRsaTransform(pModulus, &g_rsaPublicExponent, pIn, pOut, uSize);
}

unsigned int RtRsaTransform(const RtKey *pModulus,
                            const RtKey *pExponent,
                            const RtKey *pIn,
                            RtKey *pOut,
                            unsigned int uSize) {
    unsigned int uDone;

    uDone = 0;
    while (uSize >= sizeof(RtKey)) {
        RtRsaTransformBlock(pModulus, pExponent, pIn, pOut);
        uDone += sizeof(RtKey);
        uSize -= sizeof(RtKey);
        ++pIn;
        ++pOut;
    }
    return uDone;
}

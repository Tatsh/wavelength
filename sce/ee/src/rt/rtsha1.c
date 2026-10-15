#include "rt/rtsha1.h"

#include <string.h>

enum {
    kSha1BlockSize = 64,
    kSha1LengthOffset = 56,
    kSha1PadByte = 0x80,
    kSha1CountShift = 3,
    kSha1CountHighShift = 29,
    kSha1IndexMask = 0x3f,
    kSha1WordMask = 15,
    kSha1LengthHighWord = 14,
    kSha1LengthLowWord = 15,
};

#define SHA1_K1 0x5a827999U
#define SHA1_K2 0x6ed9eba1U
#define SHA1_K3 0x8f1bbcdcU
#define SHA1_K4 0xca62c1d6U

#define SHA1_F1(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define SHA1_F2(x, y, z) ((x) ^ (y) ^ (z))
#define SHA1_F3(x, y, z) (((x) & (y)) | ((z) & ((x) | (y))))
#define SHA1_F4(x, y, z) ((x) ^ (y) ^ (z))

#define SHA1_ROTL(n, x) (((x) << (n)) | ((x) >> (32 - (n))))

#define SHA1_ROUND(a, b, c, d, e, f, k, data)                                                      \
    ((e) += SHA1_ROTL(5, (a)) + f((b), (c), (d)) + (k) + (data), (b) = SHA1_ROTL(30, (b)))

// NTSC-U/C: 0x003b6b80, PAL: 0x00435a40
static RtSha1Context g_sha1Initial = {
    .auDigest = {0x67452301U, 0xefcdab89U, 0x98badcfeU, 0x10325476U, 0xc3d2e1f0U},
};

// The text whose first two bytes, read as one halfword, reveal the byte order.
static const union {
    char ac[2];
    unsigned short uWord;
} g_sha1ByteOrderProbe = {{'#', 'S'}};

// NTSC-U/C: 0x002ab574, PAL: 0x002b54e4
static void RtSha1DetectByteOrder(unsigned char *pbBigEndian) {
    if ((g_sha1ByteOrderProbe.uWord >> 8) == '#') {
        *pbBigEndian = 1;
    } else {
        *pbBigEndian = 0;
    }
}

// NTSC-U/C: 0x002ab5c8, PAL: 0x002b5538
static void RtSha1Transform(unsigned int *puDigest, const unsigned int *puData) {
    unsigned int i;
    unsigned int A;
    unsigned int B;
    unsigned int C;
    unsigned int D;
    unsigned int E;
    unsigned int auData[RT_SHA1_BLOCK_WORDS];

    A = puDigest[0];
    B = puDigest[1];
    C = puDigest[2];
    D = puDigest[3];
    E = puDigest[4];
    for (i = 0; i < RT_SHA1_BLOCK_WORDS; ++i) {
        auData[i] = puData[i];
    }

    SHA1_ROUND(A, B, C, D, E, SHA1_F1, SHA1_K1, auData[0]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F1, SHA1_K1, auData[1]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F1, SHA1_K1, auData[2]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F1, SHA1_K1, auData[3]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F1, SHA1_K1, auData[4]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F1, SHA1_K1, auData[5]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F1, SHA1_K1, auData[6]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F1, SHA1_K1, auData[7]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F1, SHA1_K1, auData[8]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F1, SHA1_K1, auData[9]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F1, SHA1_K1, auData[10]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F1, SHA1_K1, auData[11]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F1, SHA1_K1, auData[12]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F1, SHA1_K1, auData[13]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F1, SHA1_K1, auData[14]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F1, SHA1_K1, auData[15]);
    for (i = 0; i < RT_SHA1_BLOCK_WORDS; ++i) {
        auData[i & kSha1WordMask] =
            SHA1_ROTL(1,
                      auData[i & kSha1WordMask] ^ auData[(i - 14) & kSha1WordMask] ^
                          auData[(i - 8) & kSha1WordMask] ^ auData[(i - 3) & kSha1WordMask]);
    }
    SHA1_ROUND(E, A, B, C, D, SHA1_F1, SHA1_K1, auData[0]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F1, SHA1_K1, auData[1]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F1, SHA1_K1, auData[2]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F1, SHA1_K1, auData[3]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F2, SHA1_K2, auData[4]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F2, SHA1_K2, auData[5]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F2, SHA1_K2, auData[6]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F2, SHA1_K2, auData[7]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F2, SHA1_K2, auData[8]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F2, SHA1_K2, auData[9]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F2, SHA1_K2, auData[10]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F2, SHA1_K2, auData[11]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F2, SHA1_K2, auData[12]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F2, SHA1_K2, auData[13]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F2, SHA1_K2, auData[14]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F2, SHA1_K2, auData[15]);
    for (i = 0; i < RT_SHA1_BLOCK_WORDS; ++i) {
        auData[i & kSha1WordMask] =
            SHA1_ROTL(1,
                      auData[i & kSha1WordMask] ^ auData[(i - 14) & kSha1WordMask] ^
                          auData[(i - 8) & kSha1WordMask] ^ auData[(i - 3) & kSha1WordMask]);
    }
    SHA1_ROUND(D, E, A, B, C, SHA1_F2, SHA1_K2, auData[0]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F2, SHA1_K2, auData[1]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F2, SHA1_K2, auData[2]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F2, SHA1_K2, auData[3]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F2, SHA1_K2, auData[4]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F2, SHA1_K2, auData[5]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F2, SHA1_K2, auData[6]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F2, SHA1_K2, auData[7]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F3, SHA1_K3, auData[8]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F3, SHA1_K3, auData[9]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F3, SHA1_K3, auData[10]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F3, SHA1_K3, auData[11]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F3, SHA1_K3, auData[12]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F3, SHA1_K3, auData[13]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F3, SHA1_K3, auData[14]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F3, SHA1_K3, auData[15]);
    for (i = 0; i < RT_SHA1_BLOCK_WORDS; ++i) {
        auData[i & kSha1WordMask] =
            SHA1_ROTL(1,
                      auData[i & kSha1WordMask] ^ auData[(i - 14) & kSha1WordMask] ^
                          auData[(i - 8) & kSha1WordMask] ^ auData[(i - 3) & kSha1WordMask]);
    }
    SHA1_ROUND(C, D, E, A, B, SHA1_F3, SHA1_K3, auData[0]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F3, SHA1_K3, auData[1]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F3, SHA1_K3, auData[2]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F3, SHA1_K3, auData[3]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F3, SHA1_K3, auData[4]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F3, SHA1_K3, auData[5]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F3, SHA1_K3, auData[6]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F3, SHA1_K3, auData[7]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F3, SHA1_K3, auData[8]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F3, SHA1_K3, auData[9]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F3, SHA1_K3, auData[10]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F3, SHA1_K3, auData[11]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F4, SHA1_K4, auData[12]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F4, SHA1_K4, auData[13]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F4, SHA1_K4, auData[14]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F4, SHA1_K4, auData[15]);
    for (i = 0; i < RT_SHA1_BLOCK_WORDS; ++i) {
        auData[i & kSha1WordMask] =
            SHA1_ROTL(1,
                      auData[i & kSha1WordMask] ^ auData[(i - 14) & kSha1WordMask] ^
                          auData[(i - 8) & kSha1WordMask] ^ auData[(i - 3) & kSha1WordMask]);
    }
    SHA1_ROUND(B, C, D, E, A, SHA1_F4, SHA1_K4, auData[0]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F4, SHA1_K4, auData[1]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F4, SHA1_K4, auData[2]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F4, SHA1_K4, auData[3]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F4, SHA1_K4, auData[4]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F4, SHA1_K4, auData[5]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F4, SHA1_K4, auData[6]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F4, SHA1_K4, auData[7]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F4, SHA1_K4, auData[8]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F4, SHA1_K4, auData[9]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F4, SHA1_K4, auData[10]);
    SHA1_ROUND(A, B, C, D, E, SHA1_F4, SHA1_K4, auData[11]);
    SHA1_ROUND(E, A, B, C, D, SHA1_F4, SHA1_K4, auData[12]);
    SHA1_ROUND(D, E, A, B, C, SHA1_F4, SHA1_K4, auData[13]);
    SHA1_ROUND(C, D, E, A, B, SHA1_F4, SHA1_K4, auData[14]);
    SHA1_ROUND(B, C, D, E, A, SHA1_F4, SHA1_K4, auData[15]);

    puDigest[0] += A;
    puDigest[1] += B;
    puDigest[2] += C;
    puDigest[3] += D;
    puDigest[4] += E;
}

// NTSC-U/C: 0x002ade84, PAL: 0x002b7df4
static void RtSha1SwapBytes(unsigned int *puBuffer, unsigned int uSize, unsigned char bBigEndian) {
    unsigned int uValue;

    if (bBigEndian) {
        return;
    }
    uSize /= sizeof(unsigned int);
    while (uSize-- != 0) {
        uValue = *puBuffer;
        uValue = ((uValue & 0xff00ff00U) >> 8) | ((uValue & 0x00ff00ffU) << 8);
        *puBuffer++ = (uValue << 16) | (uValue >> 16);
    }
}

// NTSC-U/C: 0x002adf50, PAL: 0x002b7ec0
static void
RtSha1WriteDigest(unsigned char *pbOut, const unsigned int *puDigest, unsigned int uSize) {
    unsigned int i;
    unsigned int j;

    for (i = 0, j = 0; j < uSize; ++i, j += sizeof(unsigned int)) {
        pbOut[j + 3] = puDigest[i];
        pbOut[j + 2] = puDigest[i] >> 8;
        pbOut[j + 1] = puDigest[i] >> 16;
        pbOut[j] = puDigest[i] >> 24;
    }
}

void RtSha1Init(RtSha1Context *pContext) {
    memcpy(pContext, &g_sha1Initial, sizeof(RtSha1Context));
    RtSha1DetectByteOrder(&pContext->bBigEndian);
}

void RtSha1Update(RtSha1Context *pContext, const void *pData, unsigned int uSize) {
    const unsigned char *pbData;
    unsigned int uCount;
    unsigned int uIndex;
    unsigned char *pbBlock;

    pbData = pData;
    uCount = pContext->uCountLow;
    pContext->uCountLow = uCount + (uSize << kSha1CountShift);
    if (pContext->uCountLow < uCount) {
        ++pContext->uCountHigh;
    }
    pContext->uCountHigh += uSize >> kSha1CountHighShift;
    uIndex = (uCount >> kSha1CountShift) & kSha1IndexMask;
    if (uIndex != 0) {
        pbBlock = (unsigned char *)pContext->auData + uIndex;
        uIndex = kSha1BlockSize - uIndex;
        if (uSize < uIndex) {
            memcpy(pbBlock, pbData, uSize);
            return;
        }
        memcpy(pbBlock, pbData, uIndex);
        RtSha1SwapBytes(pContext->auData, kSha1BlockSize, pContext->bBigEndian);
        RtSha1Transform(pContext->auDigest, pContext->auData);
        pbData += uIndex;
        uSize -= uIndex;
    }
    while (uSize >= kSha1BlockSize) {
        memcpy(pContext->auData, pbData, kSha1BlockSize);
        RtSha1SwapBytes(pContext->auData, kSha1BlockSize, pContext->bBigEndian);
        RtSha1Transform(pContext->auDigest, pContext->auData);
        pbData += kSha1BlockSize;
        uSize -= kSha1BlockSize;
    }
    memcpy(pContext->auData, pbData, uSize);
}

void RtSha1Final(RtSha1Context *pContext, void *pDigest, unsigned int uDigestSize) {
    int nCount;
    unsigned char *pbBlock;

    nCount = ((int)pContext->uCountLow >> kSha1CountShift) & kSha1IndexMask;
    pbBlock = (unsigned char *)pContext->auData + nCount;
    *pbBlock++ = kSha1PadByte;
    nCount = kSha1BlockSize - 1 - nCount;
    if (nCount < 8) {
        memset(pbBlock, 0, nCount);
        RtSha1SwapBytes(pContext->auData, kSha1BlockSize, pContext->bBigEndian);
        RtSha1Transform(pContext->auDigest, pContext->auData);
        memset(pContext->auData, 0, kSha1LengthOffset);
    } else {
        memset(pbBlock, 0, nCount - 8);
    }
    pContext->auData[kSha1LengthHighWord] = pContext->uCountHigh;
    pContext->auData[kSha1LengthLowWord] = pContext->uCountLow;
    RtSha1SwapBytes(pContext->auData, kSha1LengthOffset, pContext->bBigEndian);
    RtSha1Transform(pContext->auDigest, pContext->auData);
    if (uDigestSize > RT_SHA1_DIGEST_SIZE) {
        memset(
            (unsigned char *)pDigest + RT_SHA1_DIGEST_SIZE, 0, uDigestSize - RT_SHA1_DIGEST_SIZE);
        uDigestSize = RT_SHA1_DIGEST_SIZE;
    }
    RtSha1WriteDigest(pDigest, pContext->auDigest, uDigestSize);
    memset(pContext, 0, sizeof(void *)); // Yes, the binary clears the size of a pointer only.
}

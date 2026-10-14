#ifndef RTSHA1_H
#define RTSHA1_H

#ifdef __cplusplus
extern "C" {
#endif

/** SHA-1 hashing of the SCE-RT library. */

/** The words of a SHA-1 digest. */
#define RT_SHA1_DIGEST_WORDS 5

/** The words of a SHA-1 block. */
#define RT_SHA1_BLOCK_WORDS 16

/** The size of a SHA-1 digest in bytes. */
#define RT_SHA1_DIGEST_SIZE 20

/** The state of a hash in progress. */
typedef struct {
    unsigned int auDigest[RT_SHA1_DIGEST_WORDS]; /*!< The digest so far. */
    unsigned int uCountLow;                      /*!< The low word of the bits hashed. */
    unsigned int uCountHigh;                     /*!< The high word of the bits hashed. */
    unsigned int auData[RT_SHA1_BLOCK_WORDS];    /*!< The block being filled. */
    unsigned char bBigEndian;                    /*!< Non-zero when no byte swap is needed. */
} RtSha1Context;

/**
 * Start a hash.
 *
 * @param pContext The state.
 * @ghidraAddress NTSC-U/C: 0x002ab15c
 * @ghidraAddress PAL: 0x002b50cc
 */
void RtSha1Init(RtSha1Context *pContext);

/**
 * Hash more bytes.
 *
 * @param pContext The state.
 * @param pData The bytes.
 * @param uSize The number of bytes.
 * @ghidraAddress NTSC-U/C: 0x002ab1b0
 * @ghidraAddress PAL: 0x002b5120
 */
void RtSha1Update(RtSha1Context *pContext, const void *pData, unsigned int uSize);

/**
 * Finish a hash and write its digest.
 *
 * @param pContext The state.
 * @param pDigest Receives the digest.
 * @param uDigestSize The size of the digest to write. Bytes past 20 are zero.
 * @ghidraAddress NTSC-U/C: 0x002ab3b4
 * @ghidraAddress PAL: 0x002b5324
 */
void RtSha1Final(RtSha1Context *pContext, void *pDigest, unsigned int uDigestSize);

#ifdef __cplusplus
}
#endif

#endif

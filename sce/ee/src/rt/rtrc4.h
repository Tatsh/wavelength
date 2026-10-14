#ifndef RTRC4_H
#define RTRC4_H

#include "rt/rtlargeint.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The RC4 variant of the SCE-RT library. The key schedule steps through the state by 3 or 5, and
 * the cipher feeds each plain byte back into its second index.
 */

/** The size of the state of the cipher. */
#define RT_RC4_STATE_SIZE 256

/** The state of the cipher. */
typedef struct {
    unsigned int uCount;                      /*!< The bytes processed. */
    unsigned int uI;                          /*!< The first index. */
    unsigned int uJ;                          /*!< The second index. */
    unsigned char abState[RT_RC4_STATE_SIZE]; /*!< The permutation. */
} RtRc4;

/**
 * Key the cipher.
 *
 * @param pRc4 The cipher.
 * @param pKey The key.
 * @ghidraAddress NTSC-U/C: 0x002bf0d0
 * @ghidraAddress PAL: 0x002c9040
 */
void RtRc4SetKey(RtRc4 *pRc4, const RtKey *pKey);

/**
 * Key the cipher with a salt mixed in before the key.
 *
 * @param pRc4 The cipher.
 * @param pKey The key.
 * @param uSalt The salt.
 * @ghidraAddress NTSC-U/C: 0x002bf160
 * @ghidraAddress PAL: 0x002c90d0
 */
void RtRc4SetSaltedKey(RtRc4 *pRc4, const RtKey *pKey, unsigned int uSalt);

/**
 * Encrypt a buffer in place.
 *
 * @param pRc4 The cipher.
 * @param pbData The buffer.
 * @param uSize The size of the buffer in bytes.
 * @ghidraAddress NTSC-U/C: 0x002bf248
 * @ghidraAddress PAL: 0x002c91b8
 */
void RtRc4Encrypt(RtRc4 *pRc4, unsigned char *pbData, unsigned int uSize);

/**
 * Decrypt a buffer in place.
 *
 * @param pRc4 The cipher.
 * @param pbData The buffer.
 * @param uSize The size of the buffer in bytes.
 * @ghidraAddress NTSC-U/C: 0x002bf2f0
 * @ghidraAddress PAL: 0x002c9260
 */
void RtRc4Decrypt(RtRc4 *pRc4, unsigned char *pbData, unsigned int uSize);

#ifdef __cplusplus
}
#endif

#endif

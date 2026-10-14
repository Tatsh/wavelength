#ifndef RTRSA_H
#define RTRSA_H

#include "rt/rtlargeint.h"

#ifdef __cplusplus
extern "C" {
#endif

/** RSA with 512-bit keys and the public exponent 17, for the SCE-RT library. */

/**
 * Generate a key pair from two random primes of 256 bits.
 *
 * @param pModulus Receives the modulus, the public key.
 * @param pPrivateExponent Receives the private exponent.
 * @ghidraAddress NTSC-U/C: 0x002bf398
 * @ghidraAddress PAL: 0x002c9308
 */
void RtRsaGenerateKeyPair(RtKey *pModulus, RtKey *pPrivateExponent);

/**
 * Raise each whole block of a buffer to the public exponent.
 *
 * @param pModulus The modulus.
 * @param pIn The blocks.
 * @param pOut Receives the results. It may be the same as pIn.
 * @param uSize The size of the buffer in bytes. A partial block at the end is left unprocessed.
 * @return The bytes processed.
 * @ghidraAddress NTSC-U/C: 0x002bfa7c
 * @ghidraAddress PAL: 0x002c99ec
 */
unsigned int
RtRsaEncryptPublic(const RtKey *pModulus, const RtKey *pIn, RtKey *pOut, unsigned int uSize);

/**
 * Raise each whole block of a buffer to an exponent.
 *
 * @param pModulus The modulus.
 * @param pExponent The exponent.
 * @param pIn The blocks.
 * @param pOut Receives the results. It may be the same as pIn.
 * @param uSize The size of the buffer in bytes. A partial block at the end is left unprocessed.
 * @return The bytes processed.
 * @ghidraAddress NTSC-U/C: 0x002bfab0
 * @ghidraAddress PAL: 0x002c9a20
 */
unsigned int RtRsaTransform(const RtKey *pModulus,
                            const RtKey *pExponent,
                            const RtKey *pIn,
                            RtKey *pOut,
                            unsigned int uSize);

#ifdef __cplusplus
}
#endif

#endif

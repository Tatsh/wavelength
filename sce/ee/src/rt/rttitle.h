#ifndef RTTITLE_H
#define RTTITLE_H

#include "rt/rtlargeint.h"

#ifdef __cplusplus
extern "C" {
#endif

/** The identity of the game on the SCE-RT servers, built into the program. */

/**
 * Report the identifier of the game.
 *
 * @return The identifier.
 * @ghidraAddress NTSC-U/C: 0x00323b20
 * @ghidraAddress PAL: 0x0037af1c
 */
unsigned int RtTitleGetApplicationId(void);

/**
 * Copy the RSA key the game signs its connection requests with.
 *
 * @param pModulus Receives the modulus.
 * @param pExponent Receives the exponent.
 * @ghidraAddress NTSC-U/C: 0x00323b2c
 * @ghidraAddress PAL: 0x0037af28
 */
void RtTitleGetSigningKey(RtKey *pModulus, RtKey *pExponent);

#ifdef __cplusplus
}
#endif

#endif

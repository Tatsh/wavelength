#ifndef ENCVAG_H
#define ENCVAG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start an encoding of 16-bit PCM audio to VAG data.
 *
 * @param mode The encoding mode.
 * @return Zero.
 */
int __stdcall EncVagInit(short mode);

/**
 * Encode one block of samples into one 16-byte VAG block.
 *
 * @param wav The samples.
 * @param vag Receives the block.
 * @param attribute The loop attribute of the block.
 * @return Zero.
 */
int __stdcall EncVag(short *wav, short *vag, short attribute);

/**
 * Write the 16-byte block that ends the VAG data.
 *
 * @param vag Receives the block.
 * @return Zero.
 */
int __stdcall EncVagFin(short *vag);

#ifdef __cplusplus
}
#endif

#endif

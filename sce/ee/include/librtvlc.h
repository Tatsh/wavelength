#ifndef LIBRTVLC_H
#define LIBRTVLC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Variable-length coding of the transformed blocks of pictures of the SCE-RT network library.
 *
 * The names of the routines and of their parameters are inferred.
 */

/** The mode of the coders for pictures of the media layer. */
#define RT_VLC_MODE_IMAGE 1

/**
 * Reset the encoder.
 *
 * @param mode The mode of the encoder.
 * @ghidraAddress NTSC-U/C: 0x002f2b7c
 * @ghidraAddress PAL: 0x002fcaec
 */
void rtVlcEncoderInit(int mode);

/**
 * Reset the decoder.
 *
 * @param mode The mode of the decoder.
 * @ghidraAddress NTSC-U/C: 0x002f2bc0
 * @ghidraAddress PAL: 0x002fcb30
 */
void rtVlcDecoderInit(int mode);

/**
 * Encode one quantised block of 64 coefficients.
 *
 * @param block The coefficients.
 * @param out Receives the complete bytes of the code.
 * @return The bytes written.
 * @ghidraAddress NTSC-U/C: 0x002f2c14
 * @ghidraAddress PAL: 0x002fcb84
 */
int rtVlcEncodeBlock(const short *block, unsigned char *out);

/**
 * Write the bits of the encoder that do not yet fill a byte.
 *
 * @param out Receives the bytes.
 * @return The bytes written.
 * @ghidraAddress NTSC-U/C: 0x002f3130
 * @ghidraAddress PAL: 0x002fd0a0
 */
int rtVlcEncoderFlush(unsigned char *out);

/**
 * Decode one block of 64 coefficients.
 *
 * @param block Receives the coefficients.
 * @param in The code.
 * @return The bytes read.
 * @ghidraAddress NTSC-U/C: 0x002f31c8
 * @ghidraAddress PAL: 0x002fd138
 */
int rtVlcDecodeBlock(short *block, const unsigned char *in);

#ifdef __cplusplus
}
#endif

#endif

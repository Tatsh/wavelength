#ifndef RTCODEC_H
#define RTCODEC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Speech coders of the media layer of the SCE-RT network library.
 *
 * The names of the routines and of their parameters are inferred.
 */

/** The bytes one call of rtLpcEncode() writes. */
#define RT_LPC_PACKED_SIZE 7

/** The bytes of samples one linear prediction frame covers. */
#define RT_LPC_FRAME_BYTES 360

/** The bytes one call of rtSpeechEncode() writes. */
#define RT_SPEECH_PACKED_SIZE 14

/** The bytes of samples one speech frame covers. */
#define RT_SPEECH_FRAME_BYTES 320

/** The reflection coefficients of one linear prediction frame. */
#define RT_LPC_ORDER 10

/** The frames of history the linear prediction decoder smooths over. */
#define RT_LPC_HISTORY 3

/** The pitch epochs the linear prediction synthesiser interpolates per frame, at most. */
#define RT_LPC_MAX_EPOCHS 11

/** A speech coder object. */
typedef struct rtSpeechCodec {
    unsigned char reserved[24]; /*!< Not read by the library. */ // +0x00
    void *buffer;   /*!< The buffer of the object, or null. */
    int bufferSize; /*!< The size of the buffer in bytes. */
} rtSpeechCodec;

/** The decoded pitch codes of the last three frames, the newest first. */
extern int g_anLpcPitchHistory[RT_LPC_HISTORY];

/** The decoded loudness codes of the last three frames, the newest first. */
extern int g_anLpcRmsHistory[RT_LPC_HISTORY];

/** The decoded reflection codes of the last three frames, the newest first. */
extern int g_aanLpcRcHistory[RT_LPC_HISTORY][RT_LPC_ORDER];

/** The length of each pitch epoch of the frame being synthesised. */
extern int g_anLpcEpochPitch[RT_LPC_MAX_EPOCHS];

/** The voicing of each pitch epoch of the frame being synthesised. */
extern int g_anLpcEpochVoicing[RT_LPC_MAX_EPOCHS];

/** The reflection coefficients of each pitch epoch, by coefficient and then by epoch. */
extern float g_aafLpcEpochRc[RT_LPC_ORDER][RT_LPC_MAX_EPOCHS];

/** The loudness of each pitch epoch of the frame being synthesised. */
extern float g_afLpcEpochRms[RT_LPC_MAX_EPOCHS];

/** The predictor coefficients of the pitch epoch being synthesised. */
extern float g_afLpcPredictor[RT_LPC_ORDER];

/**
 * Reset the state of the linear prediction coder.
 *
 * @ghidraAddress NTSC-U/C: 0x002eb118
 * @ghidraAddress PAL: 0x002f5088
 */
void rtLpcReset(void);

/**
 * Encode one frame of samples with the linear prediction coder.
 *
 * @param samples The RT_LPC_FRAME_BYTES bytes of samples.
 * @param packed Receives RT_LPC_PACKED_SIZE bytes.
 * @return The bytes written.
 * @ghidraAddress NTSC-U/C: 0x002eb13c
 * @ghidraAddress PAL: 0x002f50ac
 */
int rtLpcEncode(const short *samples, unsigned char *packed);

/**
 * Decode one frame of the linear prediction coder.
 *
 * @param packed The RT_LPC_PACKED_SIZE bytes of the frame.
 * @param samples Receives RT_LPC_FRAME_BYTES bytes of samples.
 * @return The samples written.
 * @ghidraAddress NTSC-U/C: 0x002eb2ec
 * @ghidraAddress PAL: 0x002f525c
 */
int rtLpcDecode(const unsigned char *packed, short *samples);

/**
 * Build the tables of the speech coder.
 *
 * @ghidraAddress NTSC-U/C: 0x002ecc8c
 * @ghidraAddress PAL: 0x002f6bfc
 */
void rtSpeechInit(void);

/**
 * Encode one frame of samples with the speech coder.
 *
 * @param samples The RT_SPEECH_FRAME_BYTES bytes of samples.
 * @param packed Receives RT_SPEECH_PACKED_SIZE bytes.
 * @return The bytes written.
 * @ghidraAddress NTSC-U/C: 0x002ece7c
 * @ghidraAddress PAL: 0x002f6dec
 */
int rtSpeechEncode(const short *samples, unsigned char *packed);

/**
 * Decode one frame of the speech coder.
 *
 * @param packed The RT_SPEECH_PACKED_SIZE bytes of the frame.
 * @param samples Receives RT_SPEECH_FRAME_BYTES bytes of samples.
 * @return The samples written.
 * @ghidraAddress NTSC-U/C: 0x002ed1bc
 * @ghidraAddress PAL: 0x002f712c
 */
int rtSpeechDecode(const unsigned char *packed, short *samples);

/**
 * Release a speech coder object.
 *
 * @param codec The object, or null.
 * @ghidraAddress NTSC-U/C: 0x002ed6e0
 * @ghidraAddress PAL: 0x002f7650
 */
void rtSpeechDestroy(rtSpeechCodec *codec);

#ifdef __cplusplus
}
#endif

#endif

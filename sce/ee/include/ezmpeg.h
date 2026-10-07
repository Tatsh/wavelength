#ifndef EZMPEG_H
#define EZMPEG_H

#include <eetypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sony's ezmpegstr sample: the audio decoder and the image loader the movie player drives, under
 * the sample's names.
 */

/**
 * One quadword of an image load DMA chain.
 *
 * A DMA tag fills the low doubleword, and a GIF tag or a register write fills all four words. The
 * DMA controller reads the chain by quadword, so every tag starts on a 16-byte boundary.
 */
typedef union {
    u_long128 qword;             /*!< The quadword. */
    unsigned long long dword[2]; /*!< The two doublewords, low first. */
    unsigned int word[4];        /*!< The four words, low first. */
} __attribute__((aligned(16))) LoadImageTag;

/** Bytes of the stream header the audio decoder collects before the samples. */
#define AUDIO_HEADER_SIZE 0x28

/** Values of AudioDec::state. */
#define AU_STATE_INIT 0   /*!< Collecting the stream header. */
#define AU_STATE_PRESET 1 /*!< Filling the IOP buffer before playback. */
#define AU_STATE_PLAY 2   /*!< Streaming to the sound processor. */
#define AU_STATE_PAUSE 3  /*!< Stopped, with the play position kept. */

/** The format block at the start of an audio stream. */
typedef struct {
    char id[4];    /*!< The block identifier. */
    int size;      /*!< Header size in bytes. */
    int type;      /*!< 0 for big-endian PCM, 1 for little-endian PCM, 2 for ADPCM. */
    int rate;      /*!< Sampling rate in hertz. */
    int ch;        /*!< Channel count. */
    int interSize; /*!< Interleave size in bytes. */
    int loopStart; /*!< Block address the interleave starts at. */
    int loopEnd;   /*!< Block address the interleave ends at. */
} SpuStreamHeader;

/** The data block header that follows the format block. */
typedef struct {
    char id[4]; /*!< The block identifier. */
    int size;   /*!< Data size in bytes. */
} SpuStreamBody;

/**
 * Audio decoder, 0x60 bytes.
 *
 * The decoder collects the stream header byte by byte, stages the samples in an Emotion Engine
 * ring, and moves them to an IOP ring the sound processor plays in a loop.
 */
typedef struct {
    int state; /*!< One of the AU_STATE values. */
    union {
        unsigned char bytes[AUDIO_HEADER_SIZE]; /*!< The header as it arrives. */
        struct {
            SpuStreamHeader sshd; /*!< The format block. */
            SpuStreamBody ssbd;   /*!< The data block header. */
        } blocks;                 /*!< The header once complete. */
    } header;                     /*!< The stream header. */
    int hdrCount;                 /*!< Header bytes collected so far. */
    unsigned char *data;          /*!< The Emotion Engine ring. */
    int put;                      /*!< Write offset into the Emotion Engine ring. */
    int count;                    /*!< Bytes staged and not yet sent. */
    int size;                     /*!< Size of the Emotion Engine ring in bytes. */
    int totalBytes;               /*!< Bytes staged since the last reset. */
    int iopBuff;                  /*!< The IOP ring. */
    int iopBuffSize;              /*!< Size of the IOP ring in bytes. */
    int iopLastPos;               /*!< Write offset into the IOP ring. */
    int iopPausePos;              /*!< Play offset the sound processor reported when stopped. */
    int totalBytesSent;           /*!< Bytes sent to the IOP since the last reset. */
    int iopZero;      /*!< A cleared IOP buffer the sound processor plays while stopped. */
    int zeroBuffSize; /*!< Size of the cleared buffer in bytes. */
} AudioDec;

/**
 * Clear an audio decoder, upload the cleared buffer to the IOP, and set the master volume.
 *
 * @param ad The decoder.
 * @param buff The Emotion Engine ring.
 * @param buffSize Size of the Emotion Engine ring in bytes.
 * @param iopBuff The IOP ring.
 * @param iopBuffSize Size of the IOP ring in bytes.
 * @param zeroBuff A buffer the routine clears and uploads.
 * @param iopZeroBuff The IOP buffer the cleared buffer is uploaded to.
 * @param zeroBuffSize Size of both cleared buffers in bytes.
 * @return Always 1.
 * @ghidraAddress NTSC-U/C: 0x001afb48
 * @ghidraAddress PAL: 0x001b88e8
 */
int audioDecCreate(AudioDec *ad,
                   unsigned char *buff,
                   int buffSize,
                   void *iopBuff,
                   int iopBuffSize,
                   unsigned char *zeroBuff,
                   void *iopZeroBuff,
                   int zeroBuffSize);

/**
 * Stop the sound processor, keep its play position, and point it at the cleared buffer.
 *
 * @param ad The decoder.
 * @ghidraAddress NTSC-U/C: 0x001afbe8
 * @ghidraAddress PAL: 0x001b8988
 */
void audioDecPause(AudioDec *ad);

/**
 * Restart the sound processor on the IOP ring from the kept play position.
 *
 * @param ad The decoder.
 * @ghidraAddress NTSC-U/C: 0x001afc70
 * @ghidraAddress PAL: 0x001b8a10
 */
void audioDecResume(AudioDec *ad);

/**
 * Start playback.
 *
 * @param ad The decoder.
 * @ghidraAddress NTSC-U/C: 0x001afce0
 * @ghidraAddress PAL: 0x001b8a80
 */
void audioDecStart(AudioDec *ad);

/**
 * Stop playback and clear the decoder back to collecting a header.
 *
 * @param ad The decoder.
 * @ghidraAddress NTSC-U/C: 0x001afd00
 * @ghidraAddress PAL: 0x001b8aa0
 */
void audioDecReset(AudioDec *ad);

/**
 * Report where the next audio bytes go, as up to two spans.
 *
 * While the header is incomplete, the first span is the rest of the header and the second is the
 * whole Emotion Engine ring. Otherwise the spans are the free part of the ring, split where the
 * ring wraps.
 *
 * @param ad The decoder.
 * @param ptr0 Receives the start of the first span.
 * @param len0 Receives the size of the first span.
 * @param ptr1 Receives the start of the second span.
 * @param len1 Receives the size of the second span.
 * @ghidraAddress NTSC-U/C: 0x001afd48
 * @ghidraAddress PAL: 0x001b8ae8
 */
void audioDecBeginPut(
    AudioDec *ad, unsigned char **ptr0, int *len0, unsigned char **ptr1, int *len1);

/**
 * Account for bytes written to the spans audioDecBeginPut() reported.
 *
 * Header bytes come first. The header is printed once it is complete, and the decoder then
 * enters AU_STATE_PRESET.
 *
 * @param ad The decoder.
 * @param size The bytes written.
 * @ghidraAddress NTSC-U/C: 0x001afdf8
 * @ghidraAddress PAL: 0x001b8b98
 */
void audioDecEndPut(AudioDec *ad, int size);

/**
 * Report whether the IOP ring has been filled once.
 *
 * @param ad The decoder.
 * @return Nonzero once the bytes sent reach the size of the IOP ring.
 * @ghidraAddress NTSC-U/C: 0x001aff10
 * @ghidraAddress PAL: 0x001b8cb0
 */
int audioDecIsPreset(AudioDec *ad);

/**
 * Move the staged bytes to the free part of the IOP ring, in whole 1024-byte blocks.
 *
 * @param ad The decoder.
 * @return The bytes sent.
 * @ghidraAddress NTSC-U/C: 0x001aff28
 * @ghidraAddress PAL: 0x001b8cc8
 */
int audioDecSendToIOP(AudioDec *ad);

/**
 * Build the DMA chain that loads a picture of 16 by 16 pixel macroblocks into the frame buffer.
 *
 * The picture is 640 pixels wide in the frame buffer, and each macroblock is 1024 bytes of 32-bit
 * pixels following the one before it.
 *
 * @param tags The chain to build.
 * @param image The macroblocks.
 * @param x Left edge in pixels.
 * @param y Top edge in pixels.
 * @param w Width in pixels.
 * @param h Height in pixels.
 * @ghidraAddress NTSC-U/C: 0x001b0428
 * @ghidraAddress PAL: 0x001b91c8
 */
void setLoadImageTags(LoadImageTag *tags, void *image, int x, int y, int w, int h);

/**
 * Build the DMA chain that fills an area of the frame buffer with one repeated macroblock.
 *
 * @param tags The chain to build.
 * @param image The macroblock.
 * @param x Left edge in pixels.
 * @param y Top edge in pixels.
 * @param w Width in pixels.
 * @param h Height in pixels.
 * @ghidraAddress NTSC-U/C: 0x001b0460
 * @ghidraAddress PAL: 0x001b9200
 */
void setLoadImageTagsTile(LoadImageTag *tags, void *image, int x, int y, int w, int h);

/**
 * Start the GIF channel on a chain setLoadImageTags() built.
 *
 * @param tags The chain.
 * @ghidraAddress NTSC-U/C: 0x001b0888
 * @ghidraAddress PAL: 0x001b9628
 */
void loadImage(LoadImageTag *tags);

/**
 * Print an error line. The game defines the routine, and the sample units call it.
 *
 * @param pszMessage The message.
 * @ghidraAddress NTSC-U/C: 0x00510f20
 * @ghidraAddress PAL: 0x00551198
 */
void ErrMessage(char *pszMessage);

#ifdef __cplusplus
}
#endif

#endif

#ifndef LIBMPEG_H
#define LIBMPEG_H

#ifdef __cplusplus
extern "C" {
#endif

/** MPEG library: program stream demultiplexing and IPU video decoding. */

struct MpegContext;

/**
 * Decoder, 0x48 bytes.
 *
 * The picture path fills the time stamps and flags of each picture it outputs.
 */
typedef struct {
    int width;                   /*!< Picture width. */
    int height;                  /*!< Picture height. */
    int frameCount;              /*!< Count of decoded frames. */
    int alignmentPadding;        /*!< Aligns the stamps. Never read or written. +0x0c */
    long long pts;               /*!< Presentation time stamp, or -1 when absent. */
    long long dts;               /*!< Decoding time stamp, or -1 when absent. */
    unsigned long long flags;    /*!< Picture header flags. */
    long long pts2nd;            /*!< Second field presentation time stamp, or -1. */
    long long dts2nd;            /*!< Second field decoding time stamp, or -1. */
    unsigned long long flags2nd; /*!< Second field picture header flags. */
    struct MpegContext *sys;     /*!< Decoder state in the work area. */
    int tailPadding;             /*!< Pads to 0x48 bytes. Never read or written. +0x44 */
} sceMpeg;

/** Stream types sceMpegAddStrCallback() accepts. */
#define sceMpegStrM2V 0   /*!< MPEG-2 video. */
#define sceMpegStrIPU 1   /*!< IPU stream. */
#define sceMpegStrPCM 2   /*!< PCM audio. */
#define sceMpegStrADPCM 3 /*!< ADPCM audio. */
#define sceMpegStrDATA 4  /*!< Data. */

/** Callback types. sceMpegAddCallback() registers the first six. */
#define sceMpegCbError 0      /*!< A decoding error. */
#define sceMpegCbNodata 1     /*!< The decoder ran out of input. */
#define sceMpegCbStopDMA 2    /*!< The decoder needs the IPU input channel stopped. */
#define sceMpegCbRestartDMA 3 /*!< The decoder releases the IPU input channel. */
#define sceMpegCbBackground 4 /*!< Idle time while the decoder waits. */
#define sceMpegCbTimeStamp 5  /*!< The decoder requests the time stamps of a picture. */
#define sceMpegCbStr 6        /*!< One demultiplexed stream packet. */

/** The callback data of an sceMpegCbError callback. */
typedef struct {
    int type;         /*!< Always sceMpegCbError. */
    char *errMessage; /*!< The error text. */
} sceMpegCbDataError;

/** The callback data of an sceMpegCbTimeStamp callback. The callback fills the stamps. */
typedef struct {
    int type;      /*!< Always sceMpegCbTimeStamp. */
    long long pts; /*!< Presentation time stamp, -1 until the callback sets it. */
    long long dts; /*!< Decoding time stamp, -1 until the callback sets it. */
} sceMpegCbDataTimeStamp;

/**
 * One demultiplexed stream packet, as sceMpegDemuxPssRing() passes it to a stream callback.
 *
 * The pointers address the input ring and may wrap at its end.
 */
typedef struct {
    int type;              /*!< Always sceMpegCbStr. */
    unsigned char *header; /*!< The packet start code. */
    unsigned char *data;   /*!< The packet payload. */
    unsigned int len;      /*!< Payload length in bytes. */
    long long pts;         /*!< Presentation time stamp, or -1 when absent. */
    long long dts;         /*!< Decoding time stamp, or -1 when absent. */
} sceMpegCbDataStr;

/** The callback data of every callback type. */
typedef union {
    int type;                         /*!< The callback type. */
    sceMpegCbDataError error;         /*!< The data of sceMpegCbError. */
    sceMpegCbDataTimeStamp timeStamp; /*!< The data of sceMpegCbTimeStamp. */
    sceMpegCbDataStr str;             /*!< The data of sceMpegCbStr. */
} sceMpegCbData;

/**
 * A decoder or stream callback.
 *
 * A stream callback returns zero to stop the demultiplexer.
 */
typedef int (*sceMpegCallback)(sceMpeg *mp, void *cbdata, void *data);

/**
 * Stop both IPU DMA channels and clear their counts, then reset the IPU and load its tables.
 *
 * @return Always 1.
 * @ghidraAddress NTSC-U/C: 0x00307f28
 * @ghidraAddress PAL: 0x003743b8
 */
int sceMpegInit(void);

/**
 * Create a decoder over a work area.
 *
 * The work area is cleared. Its decoder state takes 4288 bytes, and the stream table and the
 * picture buffers come from the rest.
 *
 * @param mp The decoder.
 * @param work_area The work area.
 * @param work_area_size Size of the work area in bytes.
 * @return 1, or 0 when the work area is too small.
 * @ghidraAddress NTSC-U/C: 0x00307fd8
 * @ghidraAddress PAL: 0x00374468
 */
int sceMpegCreate(sceMpeg *mp, unsigned char *work_area, int work_area_size);

/**
 * Delete a decoder. The decoder has nothing to release.
 *
 * @param mp The decoder.
 * @return Always 1.
 * @ghidraAddress NTSC-U/C: 0x003081e8
 * @ghidraAddress PAL: 0x00374678
 */
int sceMpegDelete(sceMpeg *mp);

/**
 * Decode pictures until one is output, colour converted into a buffer of mbcount macroblocks.
 *
 * @param mp The decoder.
 * @param rgb32 The picture buffer, aligned to 64 bytes.
 * @param mbcount Size of the buffer in macroblocks.
 * @return 1, or -1 when the picture buffer is misaligned.
 * @ghidraAddress NTSC-U/C: 0x003081f8
 * @ghidraAddress PAL: 0x00374688
 */
int sceMpegGetPicture(sceMpeg *mp, void *rgb32, int mbcount);

/**
 * Report whether the decoder has met the sequence end code.
 *
 * @param mp The decoder.
 * @return Nonzero after the sequence end code.
 * @ghidraAddress NTSC-U/C: 0x00308250
 * @ghidraAddress PAL: 0x003746e0
 */
int sceMpegIsEnd(sceMpeg *mp);

/**
 * Reset the decoder state and the IPU.
 *
 * @param mp The decoder.
 * @return Always 1.
 * @ghidraAddress NTSC-U/C: 0x00308260
 * @ghidraAddress PAL: 0x003746f0
 */
int sceMpegReset(sceMpeg *mp);

/**
 * Mark the six reference pictures as not decoded.
 *
 * @param mp The decoder.
 * @return Always 1.
 * @ghidraAddress NTSC-U/C: 0x003082b8
 * @ghidraAddress PAL: 0x00374748
 */
int sceMpegClearRefBuff(sceMpeg *mp);

/**
 * Register a decoder callback for a callback type.
 *
 * @param mp The decoder.
 * @param type The callback type.
 * @param func The callback.
 * @param data Data passed to the callback.
 * @return The previous callback.
 * @ghidraAddress NTSC-U/C: 0x00308310
 * @ghidraAddress PAL: 0x003747a0
 */
sceMpegCallback sceMpegAddCallback(sceMpeg *mp, int type, sceMpegCallback func, void *data);

/**
 * Register a stream callback for a stream type and channel.
 *
 * A duplicate key overwrites the entry in place and still increments the count.
 *
 * @param mp The decoder.
 * @param type The stream type.
 * @param ch The channel.
 * @param func The callback.
 * @param data Data passed to the callback.
 * @return The previous callback for a duplicate key, or null for a new entry.
 * @ghidraAddress NTSC-U/C: 0x003076c0
 * @ghidraAddress PAL: 0x00373b50
 */
sceMpegCallback
sceMpegAddStrCallback(sceMpeg *mp, int type, int ch, sceMpegCallback func, void *data);

/**
 * Demultiplex a PSS stream without a ring buffer.
 *
 * @param mp The decoder.
 * @param data The stream data.
 * @param size Size of the data in bytes.
 * @return The bytes consumed.
 * @ghidraAddress NTSC-U/C: 0x003076a0
 * @ghidraAddress PAL: 0x00373b30
 */
int sceMpegDemuxPss(sceMpeg *mp, unsigned char *data, int size);

/**
 * Demultiplex size bytes of a PSS stream starting at data inside a ring buffer.
 *
 * @param mp The decoder.
 * @param data The stream data, inside the ring.
 * @param size Size of the data in bytes.
 * @param buffer The ring buffer.
 * @param buffer_size Size of the ring in bytes.
 * @return The bytes consumed.
 * @ghidraAddress NTSC-U/C: 0x003073a8
 * @ghidraAddress PAL: 0x00373838
 */
int sceMpegDemuxPssRing(
    sceMpeg *mp, unsigned char *data, int size, unsigned char *buffer, int buffer_size);

#ifdef __cplusplus
}
#endif

#endif

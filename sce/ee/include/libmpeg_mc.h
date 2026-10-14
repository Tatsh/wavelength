#ifndef LIBMPEG_MC_H
#define LIBMPEG_MC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * One motion compensation job of a kernel, 28 bytes.
 *
 * The source spans two staged macroblock columns and continues into the macroblock below after
 * mRows rows.
 */
typedef struct {
    unsigned char *mDest;        /*!< Prediction output. */
    int mShift;                  /*!< Byte offset of the reference within the loaded quadword. */
    int mRows;                   /*!< Rows read before the source crosses into the next block. */
    int mRowsBelow;              /*!< Rows read from the macroblock below. */
    int mStride;                 /*!< Source row stride. */
    unsigned char *mSourceLeft;  /*!< Source in the left staged column. */
    unsigned char *mSourceRight; /*!< Source in the right staged column. */
} MpegMcDescriptor;

/** A motion compensation kernel. */
typedef void (*MpegMcKernel)(const MpegMcDescriptor *pDescriptor);

/**
 * Copy the luma prediction of a full-pel vector.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a308
 * @ghidraAddress PAL: 0x00376798
 */
void sceMpegMcLumaCopy(const MpegMcDescriptor *pDescriptor);

/**
 * Copy the chroma prediction of a full-pel vector.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a380
 * @ghidraAddress PAL: 0x00376810
 */
void sceMpegMcChromaCopy(const MpegMcDescriptor *pDescriptor);

/**
 * Interpolate the luma prediction of a vertical half-pel vector.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a418
 * @ghidraAddress PAL: 0x003768a8
 */
void sceMpegMcLumaHalfV(const MpegMcDescriptor *pDescriptor);

/**
 * Interpolate the chroma prediction of a vertical half-pel vector.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a4d0
 * @ghidraAddress PAL: 0x00376960
 */
void sceMpegMcChromaHalfV(const MpegMcDescriptor *pDescriptor);

/**
 * Interpolate the luma prediction of a horizontal half-pel vector.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a5a0
 * @ghidraAddress PAL: 0x00376a30
 */
void sceMpegMcLumaHalfH(const MpegMcDescriptor *pDescriptor);

/**
 * Interpolate the chroma prediction of a horizontal half-pel vector.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a650
 * @ghidraAddress PAL: 0x00376ae0
 */
void sceMpegMcChromaHalfH(const MpegMcDescriptor *pDescriptor);

/**
 * Interpolate the luma prediction of a diagonal half-pel vector.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a708
 * @ghidraAddress PAL: 0x00376b98
 */
void sceMpegMcLumaHalfHV(const MpegMcDescriptor *pDescriptor);

/**
 * Interpolate the chroma prediction of a diagonal half-pel vector.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a800
 * @ghidraAddress PAL: 0x00376c90
 */
void sceMpegMcChromaHalfHV(const MpegMcDescriptor *pDescriptor);

/**
 * Average a full-pel luma prediction into the prediction already written.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a900
 * @ghidraAddress PAL: 0x00376d90
 */
void sceMpegMcLumaAverageCopy(const MpegMcDescriptor *pDescriptor);

/**
 * Average a full-pel chroma prediction into the prediction already written.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030a9a0
 * @ghidraAddress PAL: 0x00376e30
 */
void sceMpegMcChromaAverageCopy(const MpegMcDescriptor *pDescriptor);

/**
 * Average a vertical half-pel luma prediction into the prediction already written.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030aa50
 * @ghidraAddress PAL: 0x00376ee0
 */
void sceMpegMcLumaAverageHalfV(const MpegMcDescriptor *pDescriptor);

/**
 * Average a vertical half-pel chroma prediction into the prediction already written.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030ab30
 * @ghidraAddress PAL: 0x00376fc0
 */
void sceMpegMcChromaAverageHalfV(const MpegMcDescriptor *pDescriptor);

/**
 * Average a horizontal half-pel luma prediction into the prediction already written.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030ac18
 * @ghidraAddress PAL: 0x003770a8
 */
void sceMpegMcLumaAverageHalfH(const MpegMcDescriptor *pDescriptor);

/**
 * Average a horizontal half-pel chroma prediction into the prediction already written.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030acf0
 * @ghidraAddress PAL: 0x00377180
 */
void sceMpegMcChromaAverageHalfH(const MpegMcDescriptor *pDescriptor);

/**
 * Average a diagonal half-pel luma prediction into the prediction already written.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030adc0
 * @ghidraAddress PAL: 0x00377250
 */
void sceMpegMcLumaAverageHalfHV(const MpegMcDescriptor *pDescriptor);

/**
 * Average a diagonal half-pel chroma prediction into the prediction already written.
 *
 * @param pDescriptor The job.
 * @ghidraAddress NTSC-U/C: 0x0030aee0
 * @ghidraAddress PAL: 0x00377370
 */
void sceMpegMcChromaAverageHalfHV(const MpegMcDescriptor *pDescriptor);

/**
 * Add a decoded block to its prediction, saturate, and pack the 384-byte macroblock.
 *
 * @param pDest The macroblock in the frame.
 * @param pPrediction The prediction.
 * @param pBlock The decoded block.
 * @ghidraAddress NTSC-U/C: 0x0030aff8
 * @ghidraAddress PAL: 0x00377488
 */
void sceMpegMcAddBlock(unsigned char *pDest,
                       const unsigned char *pPrediction,
                       const unsigned char *pBlock);

/**
 * Saturate and pack a 384-byte macroblock.
 *
 * @param pDest The macroblock in the frame.
 * @param pSource The prediction or the decoded block.
 * @ghidraAddress NTSC-U/C: 0x0030b058
 * @ghidraAddress PAL: 0x003774e8
 */
void sceMpegMcPutBlock(unsigned char *pDest, const unsigned char *pSource);

#ifdef __cplusplus
}
#endif

#endif

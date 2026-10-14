#ifndef RTGSM_H
#define RTGSM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GSM 06.10 full-rate speech coder of the media layer of the SCE-RT network library.
 *
 * The coder works on 32-bit words throughout. The names of the routines and of their parameters
 * are inferred.
 */

/** The bytes of samples one sub-band frame covers. */
#define RT_SUBBAND_FRAME_BYTES 320

/** The value of rtSubbandCodec::previousLag after a reset. */
#define RT_SUBBAND_INITIAL_LAG 40

/** The number of log-area ratio indices of one sub-band frame. */
#define RT_SUBBAND_LAR_COUNT 8

/** The number of subframes of one sub-band frame. */
#define RT_SUBBAND_SUBFRAMES 4

/** The number of pulse indices of one subframe. */
#define RT_SUBBAND_PULSES 13

/** The state of a GSM 06.10 full-rate speech coder. */
typedef struct rtSubbandCodec {
    int residual[280];       /*!< The residual that the long-term predictor reads. */
    int offsetInput;         /*!< The previous input of the offset compensation filter. */
    int offsetOutput;        /*!< The previous output of the offset compensation filter. */
    int preemphasis;         /*!< The previous sample of the pre-emphasis filter. */
    int analysisDelay[8];    /*!< The delay line of the short-term analysis filter. */
    int logAreaRatios[2][8]; /*!< The decoded log-area ratios of the last two frames. */
    int logAreaRatioRow;     /*!< The row of logAreaRatios of the current frame. */
    int previousLag;         /*!< The long-term lag of the previous decoded subframe. */
    int synthesisDelay[9];   /*!< The delay line of the short-term synthesis filter. */
    int deemphasis;          /*!< The previous output of the de-emphasis filter. */
} rtSubbandCodec;

/**
 * Analyse one frame of samples into the parameters of a GSM 06.10 frame.
 *
 * @param codec The coder.
 * @param samples The RT_SUBBAND_FRAME_BYTES bytes of samples.
 * @param logAreaRatios Receives the RT_SUBBAND_LAR_COUNT log-area ratio indices.
 * @param lags Receives the long-term lag of each subframe.
 * @param gains Receives the long-term gain index of each subframe.
 * @param gridPositions Receives the grid position of each subframe.
 * @param amplitudes Receives the block amplitude index of each subframe.
 * @param pulses Receives the RT_SUBBAND_PULSES pulse indices of each subframe.
 * @ghidraAddress NTSC-U/C: 0x002f03e8
 * @ghidraAddress PAL: 0x002fa358
 */
void rtSubbandCoder(rtSubbandCodec *codec,
                    const short *samples,
                    int *logAreaRatios,
                    int *lags,
                    int *gains,
                    int *gridPositions,
                    int *amplitudes,
                    int *pulses);

/**
 * Synthesise one frame of samples from the parameters of a GSM 06.10 frame.
 *
 * @param codec The coder.
 * @param logAreaRatios The RT_SUBBAND_LAR_COUNT log-area ratio indices.
 * @param lags The long-term lag of each subframe.
 * @param gains The long-term gain index of each subframe.
 * @param gridPositions The grid position of each subframe.
 * @param amplitudes The block amplitude index of each subframe.
 * @param pulses The RT_SUBBAND_PULSES pulse indices of each subframe.
 * @param samples Receives RT_SUBBAND_FRAME_BYTES bytes of samples.
 * @ghidraAddress NTSC-U/C: 0x002f0638
 * @ghidraAddress PAL: 0x002fa5a8
 */
void rtSubbandDecoder(rtSubbandCodec *codec,
                      int *logAreaRatios,
                      int *lags,
                      int *gains,
                      int *gridPositions,
                      int *amplitudes,
                      int *pulses,
                      short *samples);

#ifdef __cplusplus
}
#endif

#endif

#ifndef RTPITCH_H
#define RTPITCH_H

#ifdef __cplusplus
extern "C" {
#endif

/** Shared helpers of the speech codecs of the SCE-RT network library. */

/**
 * Count the shifts that normalise a 32-bit value, as the normalisation of the GSM 06.10 codec
 * counts them.
 *
 * @param value The value.
 * @return The number of left shifts that move the highest significant bit to bit 30, or 0 when
 * @p value is 0 or at most -1073741824.
 * @ghidraAddress NTSC-U/C: 0x002f9170
 * @ghidraAddress PAL: 0x003030e0
 */
int rtGsmNorm(int value);

/**
 * Compute the average magnitude difference function of the LPC-10 pitch search for a set of lags.
 *
 * Every array is indexed from zero. Each sum spans 156 samples at a stride of four, starting half
 * the gap between @p maxLag and the lag past the start.
 *
 * @param speech The speech samples.
 * @param tau The @p lagCount lags.
 * @param lagCount The number of lags.
 * @param maxLag The largest lag of the search.
 * @param amdf Receives the difference sum of each lag.
 * @param minIndex Receives the position of the smallest sum, counted from 1.
 * @param maxIndex Receives the position of the largest sum, counted from 1.
 * @ghidraAddress NTSC-U/C: 0x002f921c
 * @ghidraAddress PAL: 0x0030318c
 */
void rtLpc10Difmag(const float *speech,
                   const int *tau,
                   int lagCount,
                   int maxLag,
                   float *amdf,
                   int *minIndex,
                   int *maxIndex);

#ifdef __cplusplus
}
#endif

#endif

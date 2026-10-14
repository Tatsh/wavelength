#ifndef RTLPC10_H
#define RTLPC10_H

#include "rt/rtcodec.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The FS-1015 LPC-10 analyser and synthesiser of the SCE-RT speech coder.
 *
 * Every array parameter is indexed from zero. The analyser and the synthesiser store their state in
 * file-scope globals.
 */

/**
 * Clear the state of the LPC-10 synthesiser and of the pitch tracker.
 *
 * @ghidraAddress NTSC-U/C: 0x002f4e9c
 * @ghidraAddress PAL: 0x002fee0c
 */
void rtLpc10ResetState(void);

/**
 * Clear the buffers and the window placement of the LPC-10 analyser.
 *
 * @ghidraAddress NTSC-U/C: 0x002f5014
 * @ghidraAddress PAL: 0x002fef84
 */
void rtLpc10ResetAnalysis(void);

/**
 * Filter a frame of speech through the 100 Hz high-pass biquad cascade in place.
 *
 * @param speech The 180 samples of the frame.
 * @ghidraAddress NTSC-U/C: 0x002f4db8
 * @ghidraAddress PAL: 0x002fed28
 */
void rtLpc10Hp100(float *speech);

/**
 * Analyse a frame of speech into voicing, pitch, energy, and reflection coefficients.
 *
 * The parameters describe the frame two frames before the one passed in.
 *
 * @param speech The 180 samples of the frame.
 * @param voice Receives the voicing decisions of the two half frames.
 * @param pitch Receives the pitch period.
 * @param rms Receives the energy.
 * @param rc Receives the RT_LPC_ORDER reflection coefficients.
 * @ghidraAddress NTSC-U/C: 0x002f3770
 * @ghidraAddress PAL: 0x002fd6e0
 */
void rtLpc10Analys(const float *speech, int *voice, int *pitch, float *rms, float *rc);

/**
 * Pack the coded parameters of a frame into bits, or unpack them.
 *
 * @param direction 0 to pack, 1 to unpack.
 * @param pitchCode The code of the pitch and voicing.
 * @param rmsCode The code of the energy.
 * @param rcCode The RT_LPC_ORDER codes of the reflection coefficients.
 * @param bits The 54 bits of the frame, one per entry.
 * @ghidraAddress NTSC-U/C: 0x002f43c4
 * @ghidraAddress PAL: 0x002fe334
 */
void rtLpc10Chanwr(int direction, int *pitchCode, int *rmsCode, int *rcCode, int *bits);

/**
 * Decode an 8-bit symbol protected by a Hamming code.
 *
 * @param input The received symbol.
 * @param output Receives the corrected 4-bit value, or -1 when the symbol has an error the code
 * detects but cannot correct.
 * @ghidraAddress NTSC-U/C: 0x002f4d60
 * @ghidraAddress PAL: 0x002fecd0
 */
void rtLpc10Ham84(int input, int *output);

/**
 * Interpolate the parameters of a frame over its pitch epochs.
 *
 * @param voice The voicing decisions of the two half frames.
 * @param pitch The pitch period, updated to the pitch of the last epoch.
 * @param rms The energy, raised to at least 1.
 * @param rc The RT_LPC_ORDER reflection coefficients.
 * @param epochVoicing Receives the voicing of each epoch.
 * @param epochPitch Receives the length of each epoch.
 * @param epochRms Receives the energy of each epoch.
 * @param epochRc Receives the reflection coefficients of each epoch.
 * @param epochs Receives the count of epochs.
 * @param ratio Receives the ratio of the energy to the energy of the previous frame.
 * @ghidraAddress NTSC-U/C: 0x002f611c
 * @ghidraAddress PAL: 0x0030008c
 */
void rtLpc10Pitsyn(int *voice,
                   int *pitch,
                   float *rms,
                   float *rc,
                   int *epochVoicing,
                   int *epochPitch,
                   float *epochRms,
                   float epochRc[RT_LPC_ORDER][RT_LPC_MAX_EPOCHS],
                   int *epochs,
                   float *ratio);

/**
 * Convert the reflection coefficients of an epoch to predictor coefficients.
 *
 * @param epochRc The reflection coefficients of each epoch.
 * @param predictor Receives the RT_LPC_ORDER predictor coefficients.
 * @param gain Receives the gain of the predictor.
 * @param epoch The epoch.
 * @param gainScale The scale of the gain.
 * @ghidraAddress NTSC-U/C: 0x002f55b4
 * @ghidraAddress PAL: 0x002ff524
 */
void rtLpc10Irc2pc(float epochRc[RT_LPC_ORDER][RT_LPC_MAX_EPOCHS],
                   float *predictor,
                   float *gain,
                   int epoch,
                   float gainScale);

/**
 * Synthesise an epoch from its excitation through the predictor.
 *
 * @param predictor The RT_LPC_ORDER predictor coefficients.
 * @param pitch The length of the epoch in samples.
 * @param voicing Non-zero for a voiced epoch.
 * @param output Receives @p pitch samples.
 * @param rms The energy of the epoch.
 * @param ratio The ratio of the energy to the energy of the previous frame.
 * @param gain The gain of the predictor.
 * @ghidraAddress NTSC-U/C: 0x002f3e58
 * @ghidraAddress PAL: 0x002fddc8
 */
void rtLpc10Bsynz(const float *predictor,
                  int pitch,
                  int voicing,
                  float *output,
                  float rms,
                  float ratio,
                  float gain);

/**
 * Filter samples through the de-emphasis filter in place.
 *
 * @param samples The samples.
 * @param count The count of samples.
 * @ghidraAddress NTSC-U/C: 0x002f463c
 * @ghidraAddress PAL: 0x002fe5ac
 */
void rtLpc10Deemp(float *samples, int count);

#ifdef __cplusplus
}
#endif

#endif

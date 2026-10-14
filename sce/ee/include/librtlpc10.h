#ifndef LIBRTLPC10_H
#define LIBRTLPC10_H

#include <librtcodec.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Clear the state of the LPC-10 analyser and synthesiser.
 *
 * @ghidraAddress NTSC-U/C: 0x002f4e9c
 * @ghidraAddress PAL: 0x002fee0c
 */
void ResetSpeechCodecState(void);

/**
 * Clear the buffers of the LPC-10 analyser.
 *
 * @ghidraAddress NTSC-U/C: 0x002f5014
 * @ghidraAddress PAL: 0x002fef84
 */
void InitSpeechAnalysisBuffers(void);

/**
 * Filter a frame of speech through the pre-emphasis biquad cascade in place.
 *
 * @param speech The samples of the frame.
 * @ghidraAddress NTSC-U/C: 0x002f4db8
 * @ghidraAddress PAL: 0x002fed28
 */
void ApplyPreemphasisBiquadCascade(float *speech);

/**
 * Analyse a frame of speech into voicing, pitch, energy, and reflection coefficients.
 *
 * @param speech The samples of the frame.
 * @param voice Receives the voicing decisions of the two half frames.
 * @param pitch Receives the pitch period.
 * @param rms Receives the energy.
 * @param rc Receives the reflection coefficients.
 * @ghidraAddress NTSC-U/C: 0x002f3770
 * @ghidraAddress PAL: 0x002fd6e0
 */
void ProcessSpeechAnalysisFrame(float *speech, int *voice, int *pitch, float *rms, float *rc);

/**
 * Pack the coded parameters of a frame into bits, or unpack them.
 *
 * @param direction 0 to pack, 1 to unpack.
 * @param pitchCode The code of the pitch.
 * @param rmsCode The code of the energy.
 * @param rcCode The codes of the reflection coefficients.
 * @param bits The bits of the frame.
 * @ghidraAddress NTSC-U/C: 0x002f43c4
 * @ghidraAddress PAL: 0x002fe334
 */
void PackSpeechParameterBits(int direction, int *pitchCode, int *rmsCode, int *rcCode, int *bits);

/**
 * Decode a symbol protected by parity bits.
 *
 * @param input The received symbol.
 * @param output Receives the corrected symbol.
 * @param errors Receives the count of corrected errors.
 * @ghidraAddress NTSC-U/C: 0x002f4d60
 * @ghidraAddress PAL: 0x002fecd0
 */
void DecodeParityCheckedSymbol(int input, int *output, int *errors);

/**
 * Interpolate the parameters of a frame over its pitch epochs.
 *
 * @param voice The voicing decisions of the two half frames.
 * @param pitch The pitch period.
 * @param rms The energy.
 * @param rc The reflection coefficients.
 * @param epochVoicing Receives the voicing of each epoch.
 * @param epochPitch Receives the pitch of each epoch.
 * @param epochRms Receives the energy of each epoch.
 * @param epochRc Receives the reflection coefficients of each epoch.
 * @param epochs Receives the count of epochs.
 * @param ratio Receives the ratio of the excitation.
 * @ghidraAddress NTSC-U/C: 0x002f611c
 * @ghidraAddress PAL: 0x0030008c
 */
void InterpolateSubframeLpcParameters(int *voice,
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
 * @param predictor Receives the predictor coefficients.
 * @param gain Receives the gain of the predictor.
 * @param epoch The epoch.
 * @param gainScale The scale of the gain.
 * @ghidraAddress NTSC-U/C: 0x002f55b4
 * @ghidraAddress PAL: 0x002ff524
 */
void ConvertReflectionToLpcCoeffs(float epochRc[RT_LPC_ORDER][RT_LPC_MAX_EPOCHS],
                                  float *predictor,
                                  float *gain,
                                  int epoch,
                                  float gainScale);

/**
 * Synthesise the excitation of an epoch through the predictor.
 *
 * @param predictor The predictor coefficients.
 * @param pitch The pitch period.
 * @param voicing The voicing of the epoch.
 * @param output Receives the samples.
 * @param rms The energy.
 * @param ratio The ratio of the excitation.
 * @param gain The gain of the predictor.
 * @ghidraAddress NTSC-U/C: 0x002f3e58
 * @ghidraAddress PAL: 0x002fddc8
 */
void SynthesizeExcitationFrame(
    float *predictor, int pitch, int voicing, float *output, float rms, float ratio, float gain);

/**
 * Filter samples through the de-emphasis filter in place.
 *
 * @param samples The samples.
 * @param count The count of samples.
 * @ghidraAddress NTSC-U/C: 0x002f463c
 * @ghidraAddress PAL: 0x002fe5ac
 */
void ApplyDeemphasisFilter(float *samples, int count);

#ifdef __cplusplus
}
#endif

#endif

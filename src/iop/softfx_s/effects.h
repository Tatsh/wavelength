#pragma once

#include "softfx_s/effectparams.h"
#include "softfx_s/sampleblock.h"

/**
 * The effects. Each one reads a block of each input channel and writes a block of each output
 * channel. The module runs them in place, with each output block the same as its input block.
 */

/** Whether the stream effect mixes the stream to mono. */
extern bool g_bMonoStream;

/**
 * Apply the cubic saturation curve to a sample, clamping beyond full scale.
 *
 * @param sample The sample.
 * @return The saturated sample.
 * @ghidraAddress NTSC-U/C: 0x00001000
 * @ghidraAddress PAL: 0x00001000
 */
short Saturate(short sample);

/**
 * Pass a sample below threshold unchanged and saturate the part above it. The routine has no
 * caller.
 *
 * @param sample The sample.
 * @param threshold Magnitude where the curve starts.
 * @return The clipped sample.
 * @ghidraAddress NTSC-U/C: 0x00001078
 * @ghidraAddress PAL: 0x00001078
 */
short SoftClip(short sample, short threshold);

/**
 * Clear the delay line and the state of every effect.
 *
 * @ghidraAddress NTSC-U/C: 0x000011a0
 * @ghidraAddress PAL: 0x000011a0
 */
void InitEffects();

/**
 * Make the target parameters current and store new targets. For the swept filter, also compute
 * the steps that move the current parameters a third of the way to the targets.
 *
 * @param params The left and then the right channel's parameters.
 * @ghidraAddress NTSC-U/C: 0x000011c8
 * @ghidraAddress PAL: 0x000011c8
 */
void SetEffectParams(const EffectParams *params);

/**
 * Clear the delay line and the tracking echo's state.
 *
 * @ghidraAddress NTSC-U/C: 0x00001408
 * @ghidraAddress PAL: 0x00001408
 */
void InitDelayLine();

/**
 * Echo the mono mix through two taps of the delay line.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output, a copy of the left output.
 * @ghidraAddress NTSC-U/C: 0x00001488
 * @ghidraAddress PAL: 0x00001488
 */
void EffectEcho(SampleBlock *inLeft,
                SampleBlock *inRight,
                SampleBlock *outLeft,
                SampleBlock *outRight);

/**
 * Echo the mono mix through two taps whose delays follow the time between runs of zero
 * crossings.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output, a copy of the left output.
 * @ghidraAddress NTSC-U/C: 0x000017ac
 * @ghidraAddress PAL: 0x000017ac
 */
void EffectTrackingEcho(SampleBlock *inLeft,
                        SampleBlock *inRight,
                        SampleBlock *outLeft,
                        SampleBlock *outRight);

/**
 * Write a triangle test tone to both outputs. The routine has no caller.
 *
 * @param inLeft Left input, unused.
 * @param inRight Right input, unused.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00001bf0
 * @ghidraAddress PAL: 0x00001bf0
 */
void EffectTriangleWave(SampleBlock *inLeft,
                        SampleBlock *inRight,
                        SampleBlock *outLeft,
                        SampleBlock *outRight);

/**
 * Write a square test tone to both outputs. The routine has no caller.
 *
 * @param inLeft Left input, unused.
 * @param inRight Right input, unused.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00001cac
 * @ghidraAddress PAL: 0x00001cac
 */
void EffectSquareWave(SampleBlock *inLeft,
                      SampleBlock *inRight,
                      SampleBlock *outLeft,
                      SampleBlock *outRight);

/**
 * Copy the inputs to the outputs.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00001d2c
 * @ghidraAddress PAL: 0x00001d2c
 */
void EffectBypass(SampleBlock *inLeft,
                  SampleBlock *inRight,
                  SampleBlock *outLeft,
                  SampleBlock *outRight);

/**
 * Write each input reversed to the other channel's output. The routine has no caller.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00001d78
 * @ghidraAddress PAL: 0x00001d78
 */
void EffectReverseSwap(SampleBlock *inLeft,
                       SampleBlock *inRight,
                       SampleBlock *outLeft,
                       SampleBlock *outRight);

/**
 * Do nothing. The routine has no caller.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00001dd4
 * @ghidraAddress PAL: 0x00001dd4
 */
void EffectNone(SampleBlock *inLeft,
                SampleBlock *inRight,
                SampleBlock *outLeft,
                SampleBlock *outRight);

/**
 * Write silence to both outputs.
 *
 * @param inLeft Left input, unused.
 * @param inRight Right input, unused.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00001ddc
 * @ghidraAddress PAL: 0x00001ddc
 */
void EffectSilence(SampleBlock *inLeft,
                   SampleBlock *inRight,
                   SampleBlock *outLeft,
                   SampleBlock *outRight);

/**
 * Write one value to every sample of both outputs. The routine has no caller.
 *
 * @param inLeft Left input, unused.
 * @param inRight Right input, unused.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @param value The sample value.
 * @ghidraAddress NTSC-U/C: 0x00001e1c
 * @ghidraAddress PAL: 0x00001e1c
 */
void EffectConstant(SampleBlock *inLeft,
                    SampleBlock *inRight,
                    SampleBlock *outLeft,
                    SampleBlock *outRight,
                    short value);

/**
 * Record the EE address that receives the stream's free space.
 *
 * @param address EE address.
 * @ghidraAddress NTSC-U/C: 0x00001e50
 * @ghidraAddress PAL: 0x00001e50
 */
void SetStreamStatusAddress(void *address);

/**
 * Append data to the stream, or end the stream and discard its contents when size is zero.
 *
 * @param data The data.
 * @param size Byte count.
 * @return Zero.
 * @ghidraAddress NTSC-U/C: 0x00001e78
 * @ghidraAddress PAL: 0x00001e78
 */
int StreamData(void *data, int size);

/**
 * Play one block pair from the stream, or silence while the stream holds less than one pair.
 *
 * @param inLeft Left input, unused.
 * @param inRight Right input, unused.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00001f4c
 * @ghidraAddress PAL: 0x00001f4c
 */
void EffectStream(SampleBlock *inLeft,
                  SampleBlock *inRight,
                  SampleBlock *outLeft,
                  SampleBlock *outRight);

/**
 * Clear the ladder filter's state.
 *
 * @ghidraAddress NTSC-U/C: 0x00002238
 * @ghidraAddress PAL: 0x00002238
 */
void InitLadderFilter();

/**
 * Run the mono mix through a four-stage ladder filter and a saturator, and step the current left
 * parameters toward their targets. The routine has no caller.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00002278
 * @ghidraAddress PAL: 0x00002278
 */
void EffectLadderFilter(SampleBlock *inLeft,
                        SampleBlock *inRight,
                        SampleBlock *outLeft,
                        SampleBlock *outRight);

/**
 * Write the mono mix to both outputs, and step the current left parameters toward their
 * targets. The routine has no caller.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00002794
 * @ghidraAddress PAL: 0x00002794
 */
void EffectMonoSweep(SampleBlock *inLeft,
                     SampleBlock *inRight,
                     SampleBlock *outLeft,
                     SampleBlock *outRight);

/**
 * Pass the input only on the samples where a phase accumulator advanced by the left level wraps,
 * and silence the rest.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00002880
 * @ghidraAddress PAL: 0x00002880
 */
void EffectStutter(SampleBlock *inLeft,
                   SampleBlock *inRight,
                   SampleBlock *outLeft,
                   SampleBlock *outRight);

/**
 * Run each channel through a resonant state variable low-pass filter.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x0000291c
 * @ghidraAddress PAL: 0x0000291c
 */
void EffectResonantFilter(SampleBlock *inLeft,
                          SampleBlock *inRight,
                          SampleBlock *outLeft,
                          SampleBlock *outRight);

/**
 * Run each channel through the resonant filter and clip loud output samples to full scale.
 *
 * @param inLeft Left input.
 * @param inRight Right input.
 * @param outLeft Left output.
 * @param outRight Right output.
 * @ghidraAddress NTSC-U/C: 0x00002a58
 * @ghidraAddress PAL: 0x00002a58
 */
void EffectDistortedFilter(SampleBlock *inLeft,
                           SampleBlock *inRight,
                           SampleBlock *outLeft,
                           SampleBlock *outRight);

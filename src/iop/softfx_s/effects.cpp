#include "softfx_s/effects.h"

#include <kernel.h>
#include <sysclib.h>

#include "softfx_s/softfx.h"
#include "softfx_s/streambuffer.h"

namespace {

constexpr int kQ13Shift = 13;
constexpr short kFullScale = 0x3fff;
constexpr int kClipCeiling = 0x4000;

// One third in Q13. A swept filter reaches its target in three steps.
constexpr short kInterpolationScale = 0x0aab;
constexpr unsigned int kInterpolationSteps = 3;

// Coefficients of the cubic saturation curve in Q13.
constexpr short kSaturationCubic = 0x1000;
constexpr short kSaturationLinear = 0x3000;

constexpr short kStutterThreshold = 0x2000;
constexpr short kLadderKneeOffset = 0x1ccd;
constexpr short kLadderKnee = kStutterThreshold + kLadderKneeOffset;
constexpr short kTrackingDryGain = 0x2000;
constexpr short kTrackingWetGain = 0;
constexpr int kTrianglePeriod = 0x100;
constexpr int kTriangleAmplitude = 0x2fff;
constexpr int kSquareHalfPeriod = 24;
constexpr int kSquareAmplitude = 0x1fff;
constexpr short kDistortionThreshold = 0x1661;

// The delay line holds 2048 samples of history and the current block.
constexpr int kDelayHistory = 2048;
constexpr int kDelayLineLength = kDelayHistory + kBlockSamples;
constexpr int kTrackDelayLimit = 0x900;
constexpr int kTrackDelayFallback = 0x800;
constexpr int kTrackInitialDelay = 800;
constexpr int kTrackInitialPeriod = 200;
constexpr int kTrackInitialSample = 1;

// The ladder filter has four stages and five taps in a state of ten samples.
constexpr int kLadderStages = 4;
constexpr int kLadderTaps = kLadderStages + 1;
constexpr int kLadderStateLength = 10;
constexpr int kLadderStageOutput = kLadderStages;
constexpr int kScratchPadMode = 0;

// The stream effect plays one block of each channel at a time.
constexpr unsigned int kStreamBlockBytes = 2 * sizeof(SampleBlock);

// State of the resonant filter for one channel.
struct FilterState {
    short mLow;
    short mBand;
    short mOutput;
};

// State of the ladder filter.
struct LadderState {
    short mStage[kLadderStateLength];
};

// Layout of the scratchpad while the ladder filter runs.
struct LadderScratch {
    LadderState mInput;
    LadderState mOutput;
};

// One block of each channel as the stream holds them.
struct StereoBlock {
    SampleBlock mLeft;
    SampleBlock mRight;
};

// NTSC-U/C: 0x000051b8
EffectParams g_stepLeft;

// NTSC-U/C: 0x000051d8
EffectParams g_stepRight;

// NTSC-U/C: 0x000051f8
EffectParams g_deltaLeft;

// NTSC-U/C: 0x00005218
EffectParams g_deltaRight;

// NTSC-U/C: 0x00005238
EffectParams g_currentLeft;

// NTSC-U/C: 0x00005258
EffectParams g_currentRight;

// NTSC-U/C: 0x00005278
EffectParams g_targetLeft;

// NTSC-U/C: 0x00005298
EffectParams g_targetRight;

// NTSC-U/C: 0x000052bc
unsigned int g_nInterpolationCount;

// NTSC-U/C: 0x000052e4
int g_nTriangleCount = 0;

// NTSC-U/C: 0x000052e8
int g_nTriangleSign = 1;

// NTSC-U/C: 0x000052ec
int g_nSquareCount = 0;

// NTSC-U/C: 0x000052f0
int g_nSquareSign = 1;

// NTSC-U/C: 0x000052f4
StreamBuffer g_streamBuffer(StreamBuffer::kSize);

// NTSC-U/C: 0x00005324
void *g_pStreamBlock;

// NTSC-U/C: 0x00005328
int g_nStreamBytesPlayed;

// NTSC-U/C: 0x00006570
LadderState g_ladderOutputState;

// NTSC-U/C: 0x00006584
LadderState g_ladderInputState;

// NTSC-U/C: 0x0000532c
LadderState *g_pLadderOutput = &g_ladderOutputState;

// NTSC-U/C: 0x00005330
LadderState *g_pLadderInput = &g_ladderInputState;

// NTSC-U/C: 0x00005334
short g_nStutterPhase;

// NTSC-U/C: 0x0000533a
FilterState g_filterLeft;

// NTSC-U/C: 0x00005340
FilterState g_filterRight;

// NTSC-U/C: 0x00005348
short g_delayLine[kDelayLineLength];

// NTSC-U/C: 0x00006548
int g_trackDelay[kEchoTapCount];

// NTSC-U/C: 0x00006550
int g_trackPeriod[kEchoTapCount];

// NTSC-U/C: 0x00006558
int g_trackCrossings[kEchoTapCount];

// NTSC-U/C: 0x00006560
int g_trackAge[kEchoTapCount];

// NTSC-U/C: 0x00006568
int g_trackLastSample[kEchoTapCount];

inline int MultiplyQ13(int left, int right) {
    return (left * right) >> kQ13Shift;
}

inline short ClampSample(short sample) {
    if (sample < -kFullScale) {
        return -kFullScale;
    }
    if (sample >= kClipCeiling) {
        return kFullScale;
    }
    return sample;
}

inline int MonoMix(short left, short right) {
    return left / 2 + right / 2;
}

// Moves the delay line back one block and copies the block in after the history.
inline void ShiftDelayLine(const SampleBlock *input) {
    for (int i = 0; i < kDelayHistory; ++i) {
        g_delayLine[i] = g_delayLine[i + kBlockSamples];
    }
    memcpy(&g_delayLine[kDelayHistory], input, sizeof(SampleBlock));
}

// Moves the current left parameters one step toward their targets, three times after an update.
inline void StepCurrentParams() {
    if (g_nInterpolationCount < kInterpolationSteps) {
        g_currentLeft.mLevel = static_cast<short>(g_currentLeft.mLevel + g_stepLeft.mLevel);
        g_currentLeft.mCutoff = static_cast<short>(g_currentLeft.mCutoff + g_stepLeft.mCutoff);
        g_currentLeft.mFeedback =
            static_cast<short>(g_currentLeft.mFeedback + g_stepLeft.mFeedback);
    }
    ++g_nInterpolationCount;
}

// One sample of the resonant filter. The band and low integrators are carried in full width.
inline short
FilterSample(short input, FilterState &state, int &band, int &low, int frequency, int damping) {
    const short excitation = static_cast<short>(input - MultiplyQ13(state.mOutput, damping));
    band += MultiplyQ13(excitation - low, frequency);
    low += band;
    band = MultiplyQ13(band, damping);
    state.mOutput = static_cast<short>(low);
    return static_cast<short>(low);
}

// The saturator after the ladder filter.
inline short ShapeLadderOutput(short sample) {
    short shaped;
    short gain;
    if (sample >= 0) {
        shaped = static_cast<short>(MultiplyQ13(sample, kLadderKnee - sample));
        gain = static_cast<short>(kLadderKnee - shaped);
    } else {
        shaped = static_cast<short>(MultiplyQ13(sample, kLadderKnee + sample));
        gain = static_cast<short>(kLadderKnee + shaped);
    }
    return ClampSample(static_cast<short>(MultiplyQ13(shaped, gain)));
}

} // namespace

// NTSC-U/C: 0x000051b0
bool g_bMonoStream;

short Saturate(short sample) {
    const int magnitude = sample < 0 ? -sample : sample;
    if (magnitude < kFullScale) {
        const int cubic = MultiplyQ13(kSaturationCubic, MultiplyQ13(sample, sample));
        return static_cast<short>(MultiplyQ13(sample, kSaturationLinear - cubic));
    }
    return sample > 0 ? kFullScale : -kFullScale;
}

short SoftClip(short sample, short threshold) {
    const int magnitude = sample < 0 ? -sample : sample;
    if (magnitude < threshold) {
        return sample;
    }
    const int headroom = kFullScale - threshold;
    const int range = MultiplyQ13(headroom, kSaturationLinear);
    if (sample <= 0) {
        const int excess = (-sample - threshold) << kQ13Shift;
        const short curve = Saturate(static_cast<short>(excess / range));
        return static_cast<short>(-(threshold + MultiplyQ13(headroom, curve)));
    }
    const int excess = (sample - threshold) << kQ13Shift;
    const short curve = Saturate(static_cast<short>(excess / range));
    return static_cast<short>(threshold + MultiplyQ13(headroom, curve));
}

void InitEffects() {
    InitDelayLine();
    InitLadderFilter();
}

void SetEffectParams(const EffectParams *params) {
    g_currentLeft = g_targetLeft;
    g_currentRight = g_targetRight;
    memcpy(&g_targetLeft, &params[0], sizeof(EffectParams));
    memcpy(&g_targetRight, &params[1], sizeof(EffectParams));
    if (g_nEffect == kEffectSweptFilter) {
        const short level = static_cast<short>(g_targetLeft.mLevel - g_currentLeft.mLevel);
        g_deltaLeft.mLevel = level;
        const short cutoff = static_cast<short>(g_targetLeft.mCutoff - g_currentLeft.mCutoff);
        g_deltaLeft.mCutoff = cutoff;
        const short feedback = static_cast<short>(g_targetLeft.mFeedback - g_currentLeft.mFeedback);
        g_deltaLeft.mFeedback = feedback;
        g_deltaRight.mLevel = static_cast<short>(g_targetRight.mLevel - g_currentRight.mLevel);
        g_deltaRight.mCutoff = static_cast<short>(g_targetRight.mCutoff - g_currentRight.mCutoff);
        g_deltaRight.mFeedback =
            static_cast<short>(g_targetRight.mFeedback - g_currentRight.mFeedback);
        g_stepLeft.mLevel = static_cast<short>(MultiplyQ13(level, kInterpolationScale));
        g_stepLeft.mCutoff = static_cast<short>(MultiplyQ13(cutoff, kInterpolationScale));
        g_stepLeft.mFeedback = static_cast<short>(MultiplyQ13(feedback, kInterpolationScale));
    }
    g_nInterpolationCount = 0;
}

void InitDelayLine() {
    memset(g_delayLine, 0, sizeof(g_delayLine));
    g_trackDelay[0] = kTrackInitialDelay;
    g_trackDelay[1] = kTrackInitialDelay;
    g_trackPeriod[0] = kTrackInitialPeriod; // The second tap's period stays zero.
    g_trackCrossings[0] = 0;
    g_trackCrossings[1] = 0;
    g_trackAge[0] = 0;
    g_trackAge[1] = 0;
    g_trackLastSample[0] = kTrackInitialSample;
    g_trackLastSample[1] = kTrackInitialSample;
}

void EffectEcho(SampleBlock *inLeft,
                SampleBlock *inRight,
                SampleBlock *outLeft,
                SampleBlock *outRight) {
    ShiftDelayLine(inLeft);
    for (int i = 0; i < kBlockSamples; ++i) {
        short *current = &g_delayLine[kDelayHistory + i];
        *current = static_cast<short>(MonoMix(inLeft->mSamples[i], inRight->mSamples[i]));
        short &out = outLeft->mSamples[i];
        out = static_cast<short>(MultiplyQ13(*current, static_cast<short>(g_targetLeft.mLevel)));
        short feedback = 0;
        for (const auto &tap : g_targetLeft.mTaps) {
            const short echo = static_cast<short>(
                MultiplyQ13(current[-tap.mDelay], static_cast<short>(tap.mGain)));
            out = ClampSample(static_cast<short>(out + echo));
            feedback = ClampSample(static_cast<short>(
                feedback + MultiplyQ13(echo, static_cast<short>(g_targetLeft.mFeedback))));
        }
        outRight->mSamples[i] = out;
        *current = ClampSample(static_cast<short>(*current + feedback));
    }
}

void EffectTrackingEcho(SampleBlock *inLeft,
                        SampleBlock *inRight,
                        SampleBlock *outLeft,
                        SampleBlock *outRight) {
    ShiftDelayLine(inLeft);
    for (int i = 0; i < kBlockSamples; ++i) {
        short *current = &g_delayLine[kDelayHistory + i];
        *current = static_cast<short>(MonoMix(inLeft->mSamples[i], inRight->mSamples[i]));
        short &out = outLeft->mSamples[i];
        out = static_cast<short>(MultiplyQ13(*current, kTrackingDryGain));
        short feedback = 0;
        for (int tap = 0; tap < kEchoTapCount; ++tap) {
            const int sign = *current >= 0 ? 1 : -1;
            const int lastSign = g_trackLastSample[tap] >= 0 ? 1 : -1;
            if (sign != lastSign && ++g_trackCrossings[tap] >= g_trackPeriod[tap]) {
                g_trackDelay[tap] =
                    g_trackAge[tap] < kTrackDelayLimit ? g_trackAge[tap] : kTrackDelayFallback;
                g_trackAge[tap] = 0;
                g_trackCrossings[tap] = 0;
            }
            const short echo =
                static_cast<short>(MultiplyQ13(current[-g_trackDelay[tap]], kTrackingWetGain));
            out = ClampSample(static_cast<short>(out + echo));
            feedback = ClampSample(static_cast<short>(
                feedback + MultiplyQ13(echo, static_cast<short>(g_targetLeft.mFeedback))));
            ++g_trackAge[tap];
            g_trackLastSample[tap] = *current;
        }
        outRight->mSamples[i] = out;
        *current = ClampSample(static_cast<short>(*current + feedback));
    }
}

void EffectTriangleWave([[maybe_unused]] SampleBlock *inLeft,
                        [[maybe_unused]] SampleBlock *inRight,
                        SampleBlock *outLeft,
                        SampleBlock *outRight) {
    for (int i = 0; i < kBlockSamples; ++i) {
        if (g_nTriangleCount >= kTrianglePeriod) {
            g_nTriangleSign = -g_nTriangleSign;
            g_nTriangleCount = 0;
        }
        const short value = static_cast<short>(kTriangleAmplitude / kTrianglePeriod *
                                               g_nTriangleCount * g_nTriangleSign);
        outLeft->mSamples[i] = value;
        outRight->mSamples[i] = value;
        ++g_nTriangleCount;
    }
}

void EffectSquareWave([[maybe_unused]] SampleBlock *inLeft,
                      [[maybe_unused]] SampleBlock *inRight,
                      SampleBlock *outLeft,
                      SampleBlock *outRight) {
    for (int i = 0; i < kBlockSamples; ++i) {
        if (g_nSquareCount >= kSquareHalfPeriod) {
            g_nSquareSign = -g_nSquareSign;
            g_nSquareCount = 0;
        }
        const short value = static_cast<short>(g_nSquareSign * kSquareAmplitude);
        outLeft->mSamples[i] = value;
        outRight->mSamples[i] = value;
        ++g_nSquareCount;
    }
}

void EffectBypass(SampleBlock *inLeft,
                  SampleBlock *inRight,
                  SampleBlock *outLeft,
                  SampleBlock *outRight) {
    for (int i = 0; i < kBlockSamples; ++i) {
        outLeft->mSamples[i] = inLeft->mSamples[i];
        outRight->mSamples[i] = inRight->mSamples[i];
    }
}

void EffectReverseSwap(SampleBlock *inLeft,
                       SampleBlock *inRight,
                       SampleBlock *outLeft,
                       SampleBlock *outRight) {
    for (int i = 0; i < kBlockSamples; ++i) {
        outLeft->mSamples[kBlockSamples - 1 - i] = inRight->mSamples[i];
        outRight->mSamples[kBlockSamples - 1 - i] = inLeft->mSamples[i];
    }
}

void EffectNone([[maybe_unused]] SampleBlock *inLeft,
                [[maybe_unused]] SampleBlock *inRight,
                [[maybe_unused]] SampleBlock *outLeft,
                [[maybe_unused]] SampleBlock *outRight) {
}

void EffectSilence([[maybe_unused]] SampleBlock *inLeft,
                   [[maybe_unused]] SampleBlock *inRight,
                   SampleBlock *outLeft,
                   SampleBlock *outRight) {
    memset(outLeft, 0, sizeof(SampleBlock));
    memset(outRight, 0, sizeof(SampleBlock));
}

void EffectConstant([[maybe_unused]] SampleBlock *inLeft,
                    [[maybe_unused]] SampleBlock *inRight,
                    SampleBlock *outLeft,
                    SampleBlock *outRight,
                    short value) {
    for (int i = 0; i < kBlockSamples; ++i) {
        outLeft->mSamples[i] = value;
        outRight->mSamples[i] = value;
    }
}

void SetStreamStatusAddress(void *address) {
    g_streamBuffer.SetStatusAddress(address);
}

int StreamData(void *data, int size) {
    if (size == 0) {
        g_streamBuffer.Write(nullptr, 0, true);
        g_streamBuffer.Reset();
        g_streamBuffer.Notify();
    } else {
        g_streamBuffer.Write(data, size, false);
        g_streamBuffer.Notify();
    }
    return 0;
}

void EffectStream([[maybe_unused]] SampleBlock *inLeft,
                  [[maybe_unused]] SampleBlock *inRight,
                  SampleBlock *outLeft,
                  SampleBlock *outRight) {
    bool played = false;
    if (!g_streamBuffer.IsFinished()) {
        played = true;
        bool endOfStream;
        if (g_streamBuffer.Used(&endOfStream) < kStreamBlockBytes) {
            g_streamBuffer.Notify();
            played = false;
        } else {
            unsigned int size;
            g_pStreamBlock = g_streamBuffer.Read(kStreamBlockBytes, &size);
            g_nStreamBytesPlayed += size;
            const auto *block = static_cast<const StereoBlock *>(g_pStreamBlock);
            if (!g_bMonoStream) {
                *outLeft = block->mLeft;
                *outRight = block->mRight;
            } else {
                for (int i = 0; i < kBlockSamples; ++i) {
                    const auto mono = static_cast<short>(
                        MonoMix(block->mLeft.mSamples[i], block->mRight.mSamples[i]));
                    outLeft->mSamples[i] = mono;
                    outRight->mSamples[i] = mono;
                }
            }
            g_streamBuffer.Release(&g_pStreamBlock);
        }
    }
    if (!played) {
        memset(outLeft, 0, sizeof(SampleBlock));
        memset(outRight, 0, sizeof(SampleBlock));
    }
}

void InitLadderFilter() {
    for (int i = 0; i < kLadderTaps; ++i) {
        g_pLadderOutput->mStage[i] = 0;
        g_pLadderInput->mStage[i] = 0;
    }
}

void EffectLadderFilter(SampleBlock *inLeft,
                        SampleBlock *inRight,
                        SampleBlock *outLeft,
                        SampleBlock *outRight) {
    void *scratchPad = AllocScratchPad(kScratchPadMode);
    // The routine returns a negative error code in place of the address.
    if (reinterpret_cast<int>(scratchPad) < 0) {
        return;
    }
    auto *scratch = static_cast<LadderScratch *>(scratchPad);
    LadderState &input = scratch->mInput;
    LadderState &output = scratch->mOutput;
    input = *g_pLadderInput;
    output = *g_pLadderOutput;
    const short cutoff = static_cast<short>(g_currentLeft.mCutoff);
    const short resonance = static_cast<short>(g_currentLeft.mLevel);
    const short drive = static_cast<short>(g_currentLeft.mFeedback);
    const int mode = g_currentLeft.mMode;
    for (int i = 0; i < kBlockSamples; ++i) {
        const auto mono = static_cast<short>(MonoMix(inLeft->mSamples[i], inRight->mSamples[i]));
        if (mode != 0) {
            output.mStage[0] =
                static_cast<short>(mono - (output.mStage[kLadderStageOutput] << kQ13Shift) / drive);
        } else {
            output.mStage[0] =
                static_cast<short>(mono - MultiplyQ13(output.mStage[kLadderStageOutput], drive));
        }
        for (int stage = 0; stage < kLadderStages; ++stage) {
            output.mStage[stage + 1] =
                static_cast<short>(MultiplyQ13(output.mStage[stage], cutoff) +
                                   MultiplyQ13(input.mStage[stage], cutoff) -
                                   MultiplyQ13(output.mStage[stage + 1], resonance));
        }
        const short sample = ShapeLadderOutput(output.mStage[kLadderStageOutput]);
        for (int stage = 0; stage < kLadderStages; ++stage) {
            input.mStage[stage] = output.mStage[stage];
        }
        outLeft->mSamples[i] = sample;
        outRight->mSamples[i] = sample;
    }
    StepCurrentParams();
    *g_pLadderInput = input;
    *g_pLadderOutput = output;
    FreeScratchPad(scratchPad);
}

void EffectMonoSweep(SampleBlock *inLeft,
                     SampleBlock *inRight,
                     SampleBlock *outLeft,
                     SampleBlock *outRight) {
    for (int i = 0; i < kBlockSamples; ++i) {
        const auto mono = static_cast<short>(MonoMix(inLeft->mSamples[i], inRight->mSamples[i]));
        outLeft->mSamples[i] = mono;
        outRight->mSamples[i] = mono;
    }
    StepCurrentParams();
}

void EffectStutter(SampleBlock *inLeft,
                   SampleBlock *inRight,
                   SampleBlock *outLeft,
                   SampleBlock *outRight) {
    const auto rate = static_cast<short>(g_targetLeft.mLevel);
    for (int i = 0; i < kBlockSamples; ++i) {
        g_nStutterPhase = static_cast<short>(g_nStutterPhase + rate);
        if (g_nStutterPhase >= kStutterThreshold) {
            g_nStutterPhase = static_cast<short>(g_nStutterPhase - kStutterThreshold);
            outLeft->mSamples[i] = inLeft->mSamples[i];
            outRight->mSamples[i] = inRight->mSamples[i];
        } else {
            outLeft->mSamples[i] = 0;
            outRight->mSamples[i] = 0;
        }
    }
}

void EffectResonantFilter(SampleBlock *inLeft,
                          SampleBlock *inRight,
                          SampleBlock *outLeft,
                          SampleBlock *outRight) {
    const int frequencyLeft = g_currentLeft.mLevel;
    const int dampingLeft = g_currentLeft.mFeedback;
    const int frequencyRight = g_currentRight.mLevel;
    const int dampingRight = g_currentRight.mFeedback;
    int bandLeft = g_filterLeft.mBand;
    int lowLeft = g_filterLeft.mLow;
    int bandRight = g_filterRight.mBand;
    int lowRight = g_filterRight.mLow;
    for (int i = 0; i < kBlockSamples; ++i) {
        outLeft->mSamples[i] = FilterSample(
            inLeft->mSamples[i], g_filterLeft, bandLeft, lowLeft, frequencyLeft, dampingLeft);
        outRight->mSamples[i] = FilterSample(
            inRight->mSamples[i], g_filterRight, bandRight, lowRight, frequencyRight, dampingRight);
    }
    g_filterLeft.mBand = static_cast<short>(bandLeft);
    g_filterLeft.mLow = static_cast<short>(lowLeft);
    g_filterRight.mBand = static_cast<short>(bandRight);
    g_filterRight.mLow = static_cast<short>(lowRight);
}

void EffectDistortedFilter(SampleBlock *inLeft,
                           SampleBlock *inRight,
                           SampleBlock *outLeft,
                           SampleBlock *outRight) {
    const int frequencyLeft = g_currentLeft.mLevel;
    const int dampingLeft = g_currentLeft.mFeedback;
    const int frequencyRight = g_currentRight.mLevel;
    const int dampingRight = g_currentRight.mFeedback;
    int bandLeft = g_filterLeft.mBand;
    int lowLeft = g_filterLeft.mLow;
    int bandRight = g_filterRight.mBand;
    int lowRight = g_filterRight.mLow;
    const auto clip = [](short sample) -> short {
        const int magnitude = sample < 0 ? -sample : sample;
        if (magnitude < kDistortionThreshold) {
            return sample;
        }
        return sample >= 0 ? kFullScale : -kFullScale;
    };
    for (int i = 0; i < kBlockSamples; ++i) {
        outLeft->mSamples[i] = clip(FilterSample(
            inLeft->mSamples[i], g_filterLeft, bandLeft, lowLeft, frequencyLeft, dampingLeft));
        outRight->mSamples[i] = clip(FilterSample(inRight->mSamples[i],
                                                  g_filterRight,
                                                  bandRight,
                                                  lowRight,
                                                  frequencyRight,
                                                  dampingRight));
    }
    g_filterLeft.mBand = static_cast<short>(bandLeft);
    g_filterLeft.mLow = static_cast<short>(lowLeft);
    g_filterRight.mBand = static_cast<short>(bandRight);
    g_filterRight.mLow = static_cast<short>(lowRight);
}

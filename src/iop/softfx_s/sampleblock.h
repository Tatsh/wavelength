#pragma once

/** Samples of one channel that an effect processes at a time. */
constexpr int kBlockSamples = 256;

/** One channel's block of 16-bit samples, the unit the effects and the transfers move. */
struct SampleBlock {
    short mSamples[kBlockSamples]; /*!< Signed samples, full scale at 0x3fff after clamping. */
};

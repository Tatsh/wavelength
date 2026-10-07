#pragma once

/** Taps of the echo effects. */
constexpr int kEchoTapCount = 2;

/** One tap of the echo effect. */
struct EchoTap {
    int mDelay; /*!< Delay in samples, at most 2048. */
    int mGain;  /*!< Gain in Q13, read as a 16-bit value. */
};

/**
 * Parameters of the effects for one channel, as the EE sends them. Each effect reads the fields
 * it needs, and most read a field as a 16-bit value. The module was built without RTTI, and the
 * name is inferred.
 */
class EffectParams {
public:
    /** Clear the fields the effects share. */
    EffectParams() {
        mMode = 0;
        mCutoff = 0;
        mFeedback = 0;
        mLevel = 0;
    }

    int mMode;                    /*!< Ladder filter mode. Nonzero divides by the drive. */
    int mCutoff;                  /*!< Ladder filter cutoff in Q13. */
    int mLevel;                   /*!< Filter frequency, echo level, or stutter rate, in Q13. */
    int mFeedback;                /*!< Filter damping or echo feedback in Q13. */
    EchoTap mTaps[kEchoTapCount]; /*!< Taps of the echo effect. */
};

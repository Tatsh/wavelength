#pragma once

/**
 * Settings of the software effect, read from the `filter` entry of the first effect bus.
 *
 * The structure has no RTTI. The name is inferred. Synth::SetSoftFxFilter() sends it to the sound
 * output. The meaning of mParam and mMix depends on mType.
 */
struct SoftFxFilter {
    /** The kinds of software effect, the values of mType. */
    enum Type {
        kTypeOff = 0,             /*!< No software effect. */
        kTypeStream = 1,          /*!< The stream a StreamPlayer plays. */
        kTypeSweptFilter = 2,     /*!< A resonant filter that sweeps. */
        kTypeStutter = 3,         /*!< A gate on the input that the sweep speeds up. */
        kTypeBypass = 4,          /*!< The input unchanged. */
        kTypeFilter = 5,          /*!< A resonant filter that sweeps. */
        kTypeChorus = 6,          /*!< Two modulated delay taps, with `feedback` and `taps`. */
        kTypeFilterVariant = 7,   /*!< A resonant filter per channel that sweeps. */
        kTypeDistort = 8,         /*!< Distortion, with `distortion` and `feedback`. */
        kTypeDistortedFilter = 9, /*!< A clipped resonant filter per channel that sweeps. */
        kTypeCount = 10,          /*!< The number of types the reader accepts. */
    };

    /** The number of delay taps of a kTypeChorus effect. */
    static constexpr int kNumTaps = 2;

    /** One delay tap of a kTypeChorus effect, read from an entry of `taps`. */
    struct Tap {
        int mLfo;      /*!< `LFO`, the waveform of the modulation. */
        int mDelayMin; /*!< The first value of `delay_range`, in samples at 48 kHz. */
        int mDelayMax; /*!< The second value of `delay_range`, in samples at 48 kHz. */
        float mOscMin; /*!< The first value of `osc_range`. */
        float mOscMax; /*!< The second value of `osc_range`. */
        float mPhase;  /*!< `phase`, in whole cycles. */
        float mVolume; /*!< `volume`. */
    };

    int mType;        /*!< `type`, one of Type. */
    float mFrequency; /*!< The frequency a sweeping filter reached, recorded by the sound output. */
    int mFreqCeiling; /*!< `freq_ceiling` of a sweeping filter. */
    int mFreqFloor;   /*!< `freq_floor` of a sweeping filter. */

    /** The first parameter of the effect. */
    union {
        int mInitFreq;      /*!< `init_freq` of a sweeping filter. */
        float mAttenuation; /*!< `attenuation` of a kTypeChorus effect. */
        float mDistortion;  /*!< `distortion` of a kTypeDistort effect. */
    } mParam;

    /** The second parameter of the effect. */
    union {
        float mResonance; /*!< `resonance` of a sweeping filter. */
        float mFeedback;  /*!< `feedback` of a kTypeChorus or kTypeDistort effect. */
    } mMix;

    Tap mTaps[kNumTaps]; /*!< The taps of a kTypeChorus effect. */
};

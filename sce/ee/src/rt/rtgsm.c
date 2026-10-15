#include "rt/rtgsm.h"

#include <string.h>

#include "rt/rtpitch.h"

enum {
    kFrameSamples = 160,
    kSubframeSamples = 40,
    // The residual of the previous frames that the long-term predictor searches.
    kResidualHistory = 120,
    // The samples of the excitation buffer before and after the 40 samples of a subframe.
    kExcitationMargin = 5,
    kExcitationSize = 50,
    kGridSpacing = 3,
    kGridPositions = 4,

    kWordMin = -32768,
    kWordMax = 32767,
    kRounding15 = 0x4000,
    kShift15 = 15,
    kShift16 = 16,
};

// Limits of the long-term lag.
enum {
    kLagMin = 40,
    kLagMax = 120,
    kLagRange = kLagMax - kLagMin,
};

// Constants of the long-term parameter search.
enum {
    // The normalisation count past which the search scales the residual down.
    kLtpScaleLimit = 6,
    kLtpScaleCheck = 100,
    kLtpPowerShift = 3,
    kGainSearchSteps = 3,
    kGainMax = 3,
    // The quantised gains of the long-term predictor in units of 2^-15.
    kGain0 = 3277,
    kGain1 = 11469,
    kGain2 = 21299,
    kGain3 = 32767,
};

// Constants of the LPC analysis.
enum {
    kAutocorrelationLags = 9,
    kAutocorrelationScaleBase = 4,
    kAutocorrelationScaleMax = 4,
    kNormalizeLimit = 32,
    // The breakpoints and offsets of the piecewise conversion between reflection coefficients and
    // log-area ratios.
    kLarBreak1 = 11059,
    kLarBreak2 = 20070,
    kReflectionBreak1 = 22118,
    kReflectionBreak2 = 31130,
    kLarOffset = 26112,
    kLarSegmentShift = 2,
    // The rounding term and the shift of the log-area ratio quantiser.
    kLarRounding = 256,
    kLarQuantizeShift = 9,
    kLarBias2 = 2048,
    kLarBias3 = -2560,
    kLarBias4 = 94,
    kLarBias5 = -1792,
    kLarBias6 = -341,
    kLarBias7 = -1144,
    // The scale factors of the quantiser, each a fraction factor / 2^shift.
    kLarScale01 = 5,
    kLarScaleShift01 = 3,
    kLarScale4 = 3491,
    kLarScaleShift4 = 13,
    kLarScale5 = 15,
    kLarScaleShift5 = 5,
    kLarScale6 = 4267,
    kLarScaleShift6 = 14,
    kLarScale7 = 2259,
    kLarScaleShift7 = 13,
    // The largest magnitude index of each pair of log-area ratios.
    kLarLimit01 = 31,
    kLarLimit23 = 15,
    kLarLimit45 = 7,
    kLarLimit67 = 3,
};

// Constants of the offset compensation, pre-emphasis, and de-emphasis filters.
enum {
    kInputShift = 3,
    kInputScaleShift = 2,
    kInputMin = -0x4000,
    kInputLimit = 0x3ffd,
    kOffsetShift = 10,
    kEmphasisShift = 3,
    kOutputMin = -0x4000,
    kOutputMax = 0x3fff,
    kOutputMask = -8,
};

// Constants of the RPE coding.
enum {
    // The taps of the weighting filter in units of 2^-13.
    kWeight0 = -134,
    kWeight1 = -374,
    kWeight3 = 2054,
    kWeight4 = 5741,
    kWeightCentreShift = 13,
    kWeightRounding = 4096,
    kWeightShift = 13,
    kEnergyShift = 2,

    kBlockAmplitudeShift = 9,
    kExponentSearchSteps = 6,
    kExponentMax = 6,
    kExponentBias = 5,
    kExponentShiftLimit = 11,
    kExponentMin = -4,
    kExponentCheck = 4096,
    kMantissaShift = 3,
    kMantissaCount = 8,
    kMantissaEmpty = 7,
    kMantissaThreshold = 15,
    kShiftLimit = 16,
    kQuantizeShift = 27,
    kPulseBias = 4,
    kPulseRange = 15,
    kPulseCentre = 7,
    kPulseShift = 12,
};

// The decoded log-area ratios of each index, one table for each of the eight ratios.
// NTSC-U/C: 0x003bfa58
static const int g_anLarDecode0[] = {
    -26214, -25394, -24576, -23756, -22938, -22118, -21298, -20480, -19660, -18842, -18022,
    -17202, -16384, -15564, -14746, -13926, -13106, -12288, -11468, -10650, -9830,  -9012,
    -8192,  -7372,  -6554,  -5734,  -4916,  -4096,  -3276,  -2458,  -1638,  -820,   0,
    820,    1638,   2458,   3276,   4096,   4916,   5734,   6554,   7372,   8192,   9012,
    9830,   10650,  11468,  12288,  13108,  13926,  14746,  15564,  16384,  17202,  18022,
    18842,  19660,  20480,  21298,  22118,  22938,  23756,  24576,  25394,
};

// NTSC-U/C: 0x003bfb58
static const int g_anLarDecode1[] = {
    -26214, -25394, -24576, -23756, -22938, -22118, -21298, -20480, -19660, -18842, -18022,
    -17202, -16384, -15564, -14746, -13926, -13106, -12288, -11468, -10650, -9830,  -9012,
    -8192,  -7372,  -6554,  -5734,  -4916,  -4096,  -3276,  -2458,  -1638,  -820,   0,
    820,    1638,   2458,   3276,   4096,   4916,   5734,   6554,   7372,   8192,   9012,
    9830,   10650,  11468,  12288,  13108,  13926,  14746,  15564,  16384,  17202,  18022,
    18842,  19660,  20480,  21298,  22118,  22938,  23756,  24576,  25394,
};

// NTSC-U/C: 0x003bfc58
static const int g_anLarDecode2[] = {
    -16384, -15564, -14746, -13926, -13106, -12288, -11468, -10650, -9830, -9012, -8192,
    -7372,  -6554,  -5734,  -4916,  -4096,  -3276,  -2458,  -1638,  -820,  0,     820,
    1638,   2458,   3276,   4096,   4916,   5734,   6554,   7372,   8192,  9012,
};

// NTSC-U/C: 0x003bfcd8
static const int g_anLarDecode3[] = {
    -9012, -8192, -7372, -6554, -5734, -4916, -4096, -3276, -2458, -1638, -820,
    0,     820,   1638,  2458,  3276,  4096,  4916,  5734,  6554,  7372,  8192,
    9012,  9830,  10650, 11468, 12288, 13108, 13926, 14746, 15564, 16384,
};

// NTSC-U/C: 0x003bfd58
static const int g_anLarDecode4[] = {
    -9832,
    -8630,
    -7430,
    -6228,
    -5026,
    -3824,
    -2624,
    -1422,
    -220,
    980,
    2182,
    3384,
    4586,
    5786,
    6988,
    8190,
};

// NTSC-U/C: 0x003bfd98
static const int g_anLarDecode5[] = {
    -4916,
    -3822,
    -2730,
    -1638,
    -546,
    546,
    1638,
    2730,
    3822,
    4916,
    6008,
    7100,
    8192,
    9284,
    10376,
    11468,
};

// NTSC-U/C: 0x003bfdd8
static const int g_anLarDecode6[] = {-6554, -4588, -2622, -656, 1310, 3276, 5242, 7206};

// NTSC-U/C: 0x003bfdf8
static const int g_anLarDecode7[] = {-3278, -1422, 436, 2292, 4148, 6006, 7862, 9718};

// The decision levels of the long-term gain in units of 2^-15.
// NTSC-U/C: 0x003bfe18, PAL: 0x0043ecd8
static const int g_anPitchGainThresholds[] = {6554, 16384, 26214, 32767};

// The quantised gains of the long-term predictor in units of 2^-15.
// NTSC-U/C: 0x003bfe28
static const int g_anPitchGainFactors[] = {kGain0, kGain1, kGain2, kGain3};

// The normalising factors of the RPE quantiser for each mantissa.
// NTSC-U/C: 0x003bfe40
static const int g_anApcmNormalizeFactors[] = {
    29128,
    26215,
    23832,
    21846,
    20165,
    18725,
    17476,
    16384,
};

// The scale factors of the inverse RPE quantiser for each mantissa.
// NTSC-U/C: 0x003bfe60
static const int g_anApcmInverseFactors[] = {
    18431,
    20479,
    22527,
    24575,
    26623,
    28671,
    30719,
    32767,
};

// The excitation of the subframe being coded, with margins for the weighting filter.
// NTSC-U/C: 0x00528e58
static int g_anExcitation[kExcitationSize];

// NTSC-U/C: 0x002f05dc, PAL: 0x002fa54c
static void rtGsmPostprocess(rtSubbandCodec *codec, short *samples) {
    int deemphasis;
    int k;

    deemphasis = codec->deemphasis;
    for (k = 0; k < kFrameSamples; ++k) {
        deemphasis -= deemphasis >> kEmphasisShift;
        deemphasis += samples[k];
        if (deemphasis < kOutputMin) {
            deemphasis = kOutputMin;
        }
        if (deemphasis > kOutputMax) {
            deemphasis = kOutputMax;
        }
        samples[k] = (short)((deemphasis << 1) & kOutputMask);
    }
    codec->deemphasis = deemphasis;
}

// NTSC-U/C: 0x002f0768, PAL: 0x002fa6d8
static void
rtGsmCalculateLtpParameters(const int *residual, const int *history, int *gain, int *lag) {
    int scaled[kSubframeSamples];
    int maximum;
    int minimum;
    int value;
    int scale;
    int bestLag;
    int lambda;
    int crossMax;
    int cross;
    int power;
    int crossTop;
    int powerTop;
    int level;
    int k;

    maximum = 0;
    minimum = 0;
    for (k = 0; k < kSubframeSamples; ++k) {
        value = residual[k];
        if (maximum < value) {
            maximum = value;
        } else if (value < minimum) {
            minimum = value;
        }
    }
    minimum = -minimum;
    if (maximum < minimum) {
        maximum = minimum;
    }

    value = 0;
    if (maximum != 0) {
        value = rtGsmNorm(maximum << kShift16);
    }
    scale = 0;
    if (value <= kLtpScaleLimit) {
        scale = kLtpScaleLimit - value;
    }
    if (scale < 0) {
        return; // Yes, the binary checks a scale that cannot be negative.
    }

    for (k = 0; k < kSubframeSamples; ++k) {
        scaled[k] = residual[k] >> scale;
    }

    crossMax = 0;
    bestLag = kLagMin;
    for (lambda = kLagMin; lambda <= kLagMax; ++lambda) {
        cross = 0;
        for (k = 0; k < kSubframeSamples; ++k) {
            cross += scaled[k] * history[k - lambda];
        }
        if (crossMax < cross) {
            bestLag = lambda;
            crossMax = cross;
        }
    }
    *lag = bestLag;

    crossMax <<= 1;
    if ((unsigned int)(scale + kLtpScaleCheck) > 2 * kLtpScaleCheck) {
        return;
    }
    crossMax >>= kLtpScaleLimit - scale;
    if ((unsigned int)(bestLag - kLagMin) > kLagRange) {
        return;
    }

    power = 0;
    for (k = 0; k < kSubframeSamples; ++k) {
        value = history[k - bestLag] >> kLtpPowerShift;
        power += value * value;
    }
    power <<= 1;

    if (crossMax <= 0) {
        *gain = 0;
        return;
    }
    if (crossMax >= power) {
        *gain = kGainMax;
        return;
    }

    value = rtGsmNorm(power);
    crossTop = (crossMax << value) >> kShift16;
    powerTop = (power << value) >> kShift16;
    for (level = 0; level < kGainSearchSteps; ++level) {
        if (crossTop <= (powerTop * g_anPitchGainThresholds[level]) >> kShift15) {
            break;
        }
    }
    *gain = level;
}

// NTSC-U/C: 0x002f0f5c, PAL: 0x002faecc
static void rtGsmLongTermAnalysisFilter(
    int gain, int lag, const int *history, const int *residual, int *predicted, int *excitation) {
    int factor;
    int k;

    switch (gain) {
    case 0:
        factor = kGain0;
        break;
    case 1:
        factor = kGain1;
        break;
    case 2:
        factor = kGain2;
        break;
    case 3:
        factor = kGain3;
        break;
    default:
        return;
    }
    for (k = 0; k < kSubframeSamples; ++k) {
        predicted[k] = (factor * history[k - lag] + kRounding15) >> kShift15;
        excitation[k] = residual[k] - predicted[k];
    }
}

// NTSC-U/C: 0x002f1104, PAL: 0x002fb074
static void rtGsmLongTermPredictor(
    const int *residual, const int *history, int *excitation, int *predicted, int *lag, int *gain) {
    rtGsmCalculateLtpParameters(residual, history, gain, lag);
    rtGsmLongTermAnalysisFilter(*gain, *lag, history, residual, predicted, excitation);
}

// NTSC-U/C: 0x002f1188, PAL: 0x002fb0f8
static void rtGsmLongTermSynthesisFilter(
    rtSubbandCodec *codec, int lag, int gain, const int *excitation, int *history) {
    int factor;
    int k;

    if ((unsigned int)(lag - kLagMin) > kLagRange) {
        lag = codec->previousLag;
    }
    codec->previousLag = lag;
    if ((unsigned int)(lag - kLagMin) > kLagRange) {
        return;
    }
    factor = g_anPitchGainFactors[gain];
    if (factor == kWordMin) {
        return;
    }
    for (k = 0; k < kSubframeSamples; ++k) {
        history[k] = ((factor * history[k - lag] + kRounding15) >> kShift15) + excitation[k];
    }
    for (k = 0; k < kResidualHistory; ++k) {
        history[k - kResidualHistory] = history[k - kResidualHistory + kSubframeSamples];
    }
}

static inline void rtGsmScaleDown(int *samples, int shift) {
    int k;

    for (k = 0; k < kFrameSamples; ++k) {
        samples[k] = (samples[k] + (1 << (shift - 1))) >> shift;
    }
}

// NTSC-U/C: 0x002f1234, PAL: 0x002fb1a4
static void rtGsmAutocorrelation(int *samples, int *autocorrelation) {
    int maximum;
    int minimum;
    int value;
    int scale;
    int i;
    int k;

    maximum = 0;
    minimum = 0;
    for (k = 0; k < kFrameSamples; ++k) {
        value = samples[k];
        if (maximum < value) {
            maximum = value;
        } else if (value < minimum) {
            minimum = value;
        }
    }
    minimum = -minimum;
    if (maximum < minimum) {
        maximum = minimum;
    }

    scale = 0;
    if (maximum != 0) {
        if (maximum <= 0) {
            return; // Yes, the binary checks a maximum that the test above excludes.
        }
        scale = kAutocorrelationScaleBase - rtGsmNorm(maximum << kShift16);
    }
    switch (scale) {
    case 1:
    case 2:
    case 3:
    case 4:
        rtGsmScaleDown(samples, scale);
        break;
    default:
        break;
    }

    for (k = kAutocorrelationLags - 1; k >= 0; --k) {
        autocorrelation[k] = 0;
    }
    for (i = 0; i < kFrameSamples; ++i) {
        value = samples[i];
        for (k = 0; k <= i && k < kAutocorrelationLags; ++k) {
            autocorrelation[k] += value * samples[i - k];
        }
    }
    for (k = kAutocorrelationLags - 1; k >= 0; --k) {
        autocorrelation[k] <<= 1;
    }

    if (scale <= 0 || scale > kAutocorrelationScaleMax) {
        return;
    }
    for (k = 0; k < kFrameSamples; ++k) {
        samples[k] <<= scale;
    }
}

// NTSC-U/C: 0x002f178c, PAL: 0x002fb6fc
static void rtGsmReflectionCoefficients(const int *autocorrelation, int *reflection) {
    int normalized[kAutocorrelationLags];
    int power[kAutocorrelationLags];
    int coefficients[kAutocorrelationLags];
    int value;
    int n;
    int i;
    int m;

    if (autocorrelation[0] == 0) {
        for (i = 0; i < RT_SUBBAND_LAR_COUNT; ++i) {
            *reflection++ = 0;
        }
        return;
    }

    value = rtGsmNorm(autocorrelation[0]);
    if ((unsigned int)value >= kNormalizeLimit) {
        return;
    }
    for (i = 0; i < kAutocorrelationLags; ++i) {
        normalized[i] = (autocorrelation[i] << value) >> kShift16;
    }
    for (i = 1; i < RT_SUBBAND_LAR_COUNT; ++i) {
        coefficients[i] = normalized[i];
    }
    for (i = 0; i < kAutocorrelationLags; ++i) {
        power[i] = normalized[i];
    }

    for (n = 1; n <= RT_SUBBAND_LAR_COUNT; ++n, ++reflection) {
        value = power[1];
        if (value < 0) {
            value = -value;
        }
        if (power[0] < value) {
            for (i = n; i <= RT_SUBBAND_LAR_COUNT; ++i) {
                *reflection++ = 0;
            }
            return;
        }
        *reflection = (value << kShift15) / power[0];
        if (*reflection < 0) {
            return;
        }
        if (power[1] > 0) {
            *reflection = -*reflection;
        }
        if (*reflection == kWordMin) {
            return;
        }
        if (n == RT_SUBBAND_LAR_COUNT) {
            return;
        }

        power[0] += (power[1] * *reflection + kRounding15) >> kShift15;
        for (m = 1; m <= RT_SUBBAND_LAR_COUNT - n; ++m) {
            power[m] = ((coefficients[m] * *reflection + kRounding15) >> kShift15) + power[m + 1];
            coefficients[m] =
                ((power[m + 1] * *reflection + kRounding15) >> kShift15) + coefficients[m];
        }
    }
}

// NTSC-U/C: 0x002f19cc, PAL: 0x002fb93c
static void rtGsmTransformToLogAreaRatios(int *ratios) {
    int value;
    int i;

    for (i = 0; i < RT_SUBBAND_LAR_COUNT; ++i) {
        value = ratios[i];
        if (value < 0) {
            value = -value;
        }
        if (value < kReflectionBreak1) {
            value >>= 1;
        } else if (value < kReflectionBreak2) {
            if (value < kLarBreak1) {
                return;
            }
            value -= kLarBreak1;
        } else {
            if (value < kLarOffset) {
                return;
            }
            value = (value - kLarOffset) << kLarSegmentShift;
        }
        ratios[i] = (ratios[i] < 0) ? -value : value;
        if (ratios[i] == kWordMin) {
            return;
        }
    }
}

static inline int rtGsmQuantizeLar(int value, int limit) {
    value >>= kLarQuantizeShift;
    if (value < limit) {
        return (value < -limit) ? 0 : value + limit + 1;
    }
    return (2 * limit) + 1;
}

// NTSC-U/C: 0x002f1a50, PAL: 0x002fb9c0
static void rtGsmQuantizeLogAreaRatios(int *ratios) {
    ratios[0] = rtGsmQuantizeLar(((ratios[0] * kLarScale01) >> kLarScaleShift01) + kLarRounding,
                                 kLarLimit01);
    ratios[1] = rtGsmQuantizeLar(((ratios[1] * kLarScale01) >> kLarScaleShift01) + kLarRounding,
                                 kLarLimit01);
    ratios[2] = rtGsmQuantizeLar(
        ((ratios[2] * kLarScale01) >> kLarScaleShift01) + (kLarBias2 + kLarRounding), kLarLimit23);
    ratios[3] = rtGsmQuantizeLar(
        ((ratios[3] * kLarScale01) >> kLarScaleShift01) + (kLarBias3 + kLarRounding), kLarLimit23);
    ratios[4] = rtGsmQuantizeLar(
        ((ratios[4] * kLarScale4) >> kLarScaleShift4) + (kLarBias4 + kLarRounding), kLarLimit45);
    ratios[5] = rtGsmQuantizeLar(
        ((ratios[5] * kLarScale5) >> kLarScaleShift5) + (kLarBias5 + kLarRounding), kLarLimit45);
    ratios[6] = rtGsmQuantizeLar(
        ((ratios[6] * kLarScale6) >> kLarScaleShift6) + (kLarBias6 + kLarRounding), kLarLimit67);
    ratios[7] = rtGsmQuantizeLar(
        ((ratios[7] * kLarScale7) >> kLarScaleShift7) + (kLarBias7 + kLarRounding), kLarLimit67);
}

// NTSC-U/C: 0x002f1c48, PAL: 0x002fbbb8
static void rtGsmLpcAnalysis(int *samples, int *logAreaRatios) {
    int autocorrelation[kAutocorrelationLags];

    rtGsmAutocorrelation(samples, autocorrelation);
    rtGsmReflectionCoefficients(autocorrelation, logAreaRatios);
    rtGsmTransformToLogAreaRatios(logAreaRatios);
    rtGsmQuantizeLogAreaRatios(logAreaRatios);
}

// NTSC-U/C: 0x002f1c8c, PAL: 0x002fbbfc
static void rtGsmPreprocess(rtSubbandCodec *codec, const short *samples, int *output) {
    int offsetInput;
    int offsetOutput;
    int preemphasis;
    int sample;
    int difference;
    int emphasis;
    int k;

    offsetInput = codec->offsetInput;
    offsetOutput = codec->offsetOutput;
    preemphasis = codec->preemphasis;
    for (k = 0; k < kFrameSamples; ++k) {
        sample = (samples[k] >> kInputShift) << kInputScaleShift;
        if (sample < kInputMin) {
            return;
        }
        if (sample >= kInputLimit) {
            return;
        }
        difference = sample - offsetInput;
        if (difference == kWordMin) {
            return;
        }
        offsetInput = sample;

        offsetOutput += (difference << kShift15) -
                        ((offsetOutput >> kOffsetShift) - (offsetOutput >> kShift15));
        emphasis = (preemphasis >> kEmphasisShift) - preemphasis;
        preemphasis = (offsetOutput + kRounding15) >> kShift15;
        output[k] = preemphasis + emphasis;
    }
    codec->offsetInput = offsetInput;
    codec->offsetOutput = offsetOutput;
    codec->preemphasis = preemphasis;
}

// NTSC-U/C: 0x002f1d28, PAL: 0x002fbc98
static void rtGsmWeightingFilter(const int *excitation, int *weighted) {
    const int *window;
    int result;
    int k;

    for (k = 0; k < kSubframeSamples; ++k) {
        // Yes, the binary runs an empty loop of 40 steps here.
    }
    for (k = 0; k < kSubframeSamples; ++k) {
        window = &excitation[k - kExcitationMargin];
        result = (window[7] * kWeight3) + (window[9] * kWeight1) +
                 (window[5] << kWeightCentreShift) + (window[6] * kWeight4) +
                 (window[3] * kWeight3) + (window[4] * kWeight4) + (window[0] * kWeight0) +
                 (window[1] * kWeight1) + (window[10] * kWeight0);
        result = (result + kWeightRounding) >> kWeightShift;
        if (result < kWordMin) {
            result = kWordMin;
        } else if (result > kWordMax) {
            result = kWordMax;
        }
        weighted[k] = result;
    }
    for (k = 0; k < kSubframeSamples; ++k) {
        // Yes, the binary runs a second empty loop of 40 steps here.
    }
}

static inline int rtGsmGridEnergy(const int *weighted, int first, int last) {
    int energy;
    int value;
    int i;

    energy = 0;
    for (i = first; i <= last; i += kGridSpacing) {
        value = weighted[i] >> kEnergyShift;
        energy += value * value;
    }
    return energy;
}

// NTSC-U/C: 0x002f1e64, PAL: 0x002fbdd4
static void rtGsmRpeGridSelection(const int *weighted, int *selected, int *gridPosition) {
    int common;
    int bestEnergy;
    int energy;
    int position;
    int i;

    common = rtGsmGridEnergy(weighted, kGridSpacing, (RT_SUBBAND_PULSES - 1) * kGridSpacing);
    bestEnergy = (common + rtGsmGridEnergy(weighted, 0, 0)) << 1;
    position = 0;

    energy = rtGsmGridEnergy(weighted, 1, 1 + (RT_SUBBAND_PULSES - 1) * kGridSpacing) << 1;
    if (bestEnergy < energy) {
        position = 1;
        bestEnergy = energy;
    }
    energy = rtGsmGridEnergy(weighted, 2, 2 + (RT_SUBBAND_PULSES - 1) * kGridSpacing) << 1;
    if (bestEnergy < energy) {
        position = 2;
        bestEnergy = energy;
    }
    energy = (rtGsmGridEnergy(weighted, kSubframeSamples - 1, kSubframeSamples - 1) + common) << 1;
    if (bestEnergy < energy) {
        position = 3;
    }

    for (i = 0; i < RT_SUBBAND_PULSES; ++i) {
        selected[i] = weighted[position + (kGridSpacing * i)];
    }
    *gridPosition = position;
}

// NTSC-U/C: 0x002f2160, PAL: 0x002fc0d0
static void rtGsmApcmExponentMantissa(int amplitude, int *exponent, int *mantissa) {
    int exponentValue;
    int mantissaValue;

    exponentValue = 0;
    if (amplitude > kMantissaThreshold) {
        exponentValue = (amplitude >> kMantissaShift) - 1;
    }
    mantissaValue = amplitude - (exponentValue << kMantissaShift);
    if (mantissaValue == 0) {
        exponentValue = kExponentMin;
        mantissaValue = kMantissaEmpty;
    } else {
        while (mantissaValue < kMantissaCount) {
            mantissaValue = (mantissaValue << 1) | 1;
            --exponentValue;
        }
        mantissaValue -= kMantissaCount;
    }
    if ((unsigned int)(exponentValue - kExponentMin) >
        (unsigned int)(kExponentMax - kExponentMin)) {
        return;
    }
    if ((unsigned int)mantissaValue >= kMantissaCount) {
        return;
    }
    *exponent = exponentValue;
    *mantissa = mantissaValue;
}

// NTSC-U/C: 0x002f21d8, PAL: 0x002fc148
static void rtGsmApcmQuantize(
    const int *selected, int *pulses, int *mantissaOut, int *exponentOut, int *amplitudeOut) {
    int maximum;
    int value;
    int test;
    int exponent;
    int mantissa;
    int amplitude;
    int shift;
    int factor;
    int i;

    maximum = 0;
    for (i = 0; i < RT_SUBBAND_PULSES; ++i) {
        value = selected[i];
        if (value < 0) {
            value = -value;
        }
        if (maximum < value) {
            maximum = value;
        }
    }
    if (maximum > kWordMax) {
        maximum = kWordMax;
    }

    exponent = 0;
    value = maximum >> kBlockAmplitudeShift;
    test = 0;
    for (i = 0; i < kExponentSearchSteps; ++i) {
        test |= (value <= 0);
        if (exponent >= kExponentMax) {
            return;
        }
        value >>= 1;
        if (test == 0) {
            ++exponent;
        }
    }
    if ((unsigned int)exponent > kExponentMax) {
        return;
    }
    value = exponent + kExponentBias;
    if ((unsigned int)value > kExponentShiftLimit) {
        return;
    }
    amplitude = (maximum >> value) + (exponent << kMantissaShift);

    rtGsmApcmExponentMantissa(amplitude, &exponent, &mantissa);
    if ((unsigned int)(exponent + kExponentCheck) > 2 * kExponentCheck) {
        return;
    }
    if ((unsigned int)mantissa >= kMantissaCount) {
        return;
    }

    shift = kExponentMax - exponent;
    factor = g_anApcmNormalizeFactors[mantissa];
    for (i = 0; i < RT_SUBBAND_PULSES; ++i) {
        if ((unsigned int)shift >= kShiftLimit) {
            return;
        }
        value = selected[i] << shift;
        pulses[i] = ((value * factor) >> kQuantizeShift) + kPulseBias;
    }
    *mantissaOut = mantissa;
    *exponentOut = exponent;
    *amplitudeOut = amplitude;
}

// Yes, the binary reads the mantissa and the exponent of rtGsmRpeEncode() uninitialised when the
// quantiser returns early.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

// NTSC-U/C: 0x002f2380, PAL: 0x002fc2f0
static void
rtGsmApcmInverseQuantize(const int *pulses, int mantissa, int exponent, int *dequantized) {
    int factor;
    int shift;
    int rounding;
    int value;
    int i;

    if ((unsigned int)mantissa >= kMantissaCount) {
        return;
    }
    factor = g_anApcmInverseFactors[mantissa];
    shift = kExponentMax - exponent;
    rounding = 0;
    if (shift > 0) {
        rounding = 1 << (shift - 1);
    }
    for (i = 0; i < RT_SUBBAND_PULSES; ++i) {
        value = pulses[i];
        if ((unsigned int)value >= kMantissaCount) {
            return;
        }
        if ((unsigned int)(value * 2) >= kPulseRange) {
            return;
        }
        value = ((value * 2) - kPulseCentre) << kPulseShift;
        value = (factor * value + kRounding15) >> kShift15;
        dequantized[i] = (value + rounding) >> shift;
    }
}

// NTSC-U/C: 0x002f240c, PAL: 0x002fc37c
static void rtGsmRpeGridPositioning(int gridPosition, const int *dequantized, int *excitation) {
    int i;

    if ((unsigned int)gridPosition >= kGridPositions) {
        return;
    }
    i = RT_SUBBAND_PULSES;
    switch (gridPosition) {
    case 3:
        *excitation++ = 0;
        /* fall through */
    case 2:
        do {
            *excitation++ = 0;
            /* fall through */
        case 1:
            *excitation++ = 0;
            /* fall through */
        case 0:
            *excitation++ = *dequantized++;
        } while (--i);
        break;
    default:
        break;
    }
    while (++gridPosition < kGridPositions) {
        *excitation++ = 0;
    }
}

// NTSC-U/C: 0x002f24a8, PAL: 0x002fc418
static void rtGsmRpeEncode(int *excitation, int *amplitude, int *gridPosition, int *pulses) {
    int weighted[kSubframeSamples];
    int selected[RT_SUBBAND_PULSES];
    int dequantized[RT_SUBBAND_PULSES];
    int mantissa;
    int exponent;

    rtGsmWeightingFilter(excitation, weighted);
    rtGsmRpeGridSelection(weighted, selected, gridPosition);
    rtGsmApcmQuantize(selected, pulses, &mantissa, &exponent, amplitude);
    rtGsmApcmInverseQuantize(pulses, mantissa, exponent, dequantized);
    rtGsmRpeGridPositioning(*gridPosition, dequantized, excitation);
}

#pragma GCC diagnostic pop

// NTSC-U/C: 0x002f2550, PAL: 0x002fc4c0
static void rtGsmRpeDecode(int amplitude, int gridPosition, const int *pulses, int *excitation) {
    int dequantized[RT_SUBBAND_PULSES];
    int exponent;
    int mantissa;

    rtGsmApcmExponentMantissa(amplitude, &exponent, &mantissa);
    rtGsmApcmInverseQuantize(pulses, mantissa, exponent, dequantized);
    rtGsmRpeGridPositioning(gridPosition, dequantized, excitation);
}

// NTSC-U/C: 0x002f25b8, PAL: 0x002fc528
static void rtGsmDecodeLogAreaRatios(const int *indices, int *ratios) {
    ratios[0] = g_anLarDecode0[indices[0]];
    ratios[1] = g_anLarDecode1[indices[1]];
    ratios[2] = g_anLarDecode2[indices[2]];
    ratios[3] = g_anLarDecode3[indices[3]];
    ratios[4] = g_anLarDecode4[indices[4]];
    ratios[5] = g_anLarDecode5[indices[5]];
    ratios[6] = g_anLarDecode6[indices[6]];
    ratios[7] = g_anLarDecode7[indices[7]];
}

// NTSC-U/C: 0x002f267c, PAL: 0x002fc5ec
static void rtGsmLarCoefficients0To12(const int *previous, const int *current, int *ratios) {
    int value;
    int i;

    for (i = 0; i < RT_SUBBAND_LAR_COUNT; ++i) {
        value = (previous[i] >> 2) + (current[i] >> 2);
        ratios[i] = value; // Yes, the binary stores the partial sum first.
        ratios[i] = (previous[i] >> 1) + value;
    }
}

// NTSC-U/C: 0x002f26c8, PAL: 0x002fc638
static void rtGsmLarCoefficients13To26(const int *previous, const int *current, int *ratios) {
    int i;

    for (i = 0; i < RT_SUBBAND_LAR_COUNT; ++i) {
        ratios[i] = (previous[i] >> 1) + (current[i] >> 1);
    }
}

// NTSC-U/C: 0x002f2704, PAL: 0x002fc674
static void rtGsmLarCoefficients27To39(const int *previous, const int *current, int *ratios) {
    int value;
    int i;

    for (i = 0; i < RT_SUBBAND_LAR_COUNT; ++i) {
        value = (previous[i] >> 2) + (current[i] >> 2);
        ratios[i] = value; // Yes, the binary stores the partial sum first.
        ratios[i] = (current[i] >> 1) + value;
    }
}

// NTSC-U/C: 0x002f2750, PAL: 0x002fc6c0
static void rtGsmLarCoefficients40To159(const int *current, int *ratios) {
    int i;

    for (i = 0; i < RT_SUBBAND_LAR_COUNT; ++i) {
        ratios[i] = current[i];
    }
}

// NTSC-U/C: 0x002f2778, PAL: 0x002fc6e8
static void rtGsmLarToReflection(int *ratios) {
    int value;
    int i;

    for (i = 0; i < RT_SUBBAND_LAR_COUNT; ++i) {
        if (ratios[i] < 0) {
            value = (ratios[i] == kWordMin) ? kWordMax : -ratios[i];
            if (value < kLarBreak1) {
                value <<= 1;
            } else if (value < kLarBreak2) {
                value += kLarBreak1;
            } else {
                value = (value >> kLarSegmentShift) + kLarOffset;
            }
            ratios[i] = -value;
        } else {
            value = ratios[i];
            if (value < kLarBreak1) {
                value <<= 1;
            } else if (value < kLarBreak2) {
                value += kLarBreak1;
            } else {
                value = (value >> kLarSegmentShift) + kLarOffset;
            }
            ratios[i] = value;
        }
    }
}

// NTSC-U/C: 0x002f2810, PAL: 0x002fc780
static void rtGsmShortTermAnalysisFiltering(rtSubbandCodec *codec,
                                            const int *reflection,
                                            int count,
                                            int *samples) {
    int *delay;
    int forward;
    int backward;
    int previous;
    int coefficient;
    int i;

    delay = codec->analysisDelay;
    for (; count != 0; --count, ++samples) {
        backward = *samples;
        forward = backward;
        for (i = 0; i < RT_SUBBAND_LAR_COUNT; ++i) {
            previous = delay[i];
            coefficient = reflection[i];
            delay[i] = backward;
            backward = previous + ((coefficient * forward + kRounding15) >> kShift15);
            forward += (coefficient * previous + kRounding15) >> kShift15;
        }
        *samples = forward;
    }
}

// NTSC-U/C: 0x002f2890, PAL: 0x002fc800
static void rtGsmShortTermSynthesisFiltering(
    rtSubbandCodec *codec, const int *reflection, int count, const int *input, short *output) {
    int *delay;
    int sample;
    int coefficient;
    int term;
    int i;

    delay = codec->synthesisDelay;
    for (; count != 0; --count) {
        sample = *input++;
        for (i = RT_SUBBAND_LAR_COUNT - 1; i >= 0; --i) {
            coefficient = reflection[i];
            term = delay[i];
            if (coefficient == kWordMin && term == kWordMin) {
                term = kWordMax;
            } else {
                term = (coefficient * term + kRounding15) >> kShift15;
            }
            sample -= term;
            if (coefficient == kWordMin && sample == kWordMin) {
                term = kWordMax;
            } else {
                term = (coefficient * sample + kRounding15) >> kShift15;
            }
            delay[i + 1] = term + delay[i];
        }
        delay[0] = sample;
        *output++ = (short)sample;
    }
}

// The subframes of the short-term filters, each with its own interpolated reflection coefficients.
enum {
    kShortTermFirst = 0,
    kShortTermFirstCount = 13,
    kShortTermSecond = 13,
    kShortTermSecondCount = 14,
    kShortTermThird = 27,
    kShortTermThirdCount = 13,
    kShortTermFourth = 40,
    kShortTermFourthCount = 120,
};

// NTSC-U/C: 0x002f2930, PAL: 0x002fc8a0
static void
rtGsmShortTermAnalysisFilter(rtSubbandCodec *codec, const int *logAreaRatios, int *samples) {
    int ratios[RT_SUBBAND_LAR_COUNT];
    int *current;
    int *previous;

    current = codec->logAreaRatios[codec->logAreaRatioRow];
    codec->logAreaRatioRow ^= 1;
    previous = codec->logAreaRatios[codec->logAreaRatioRow];

    rtGsmDecodeLogAreaRatios(logAreaRatios, current);

    rtGsmLarCoefficients0To12(previous, current, ratios);
    rtGsmLarToReflection(ratios);
    rtGsmShortTermAnalysisFiltering(codec, ratios, kShortTermFirstCount, &samples[kShortTermFirst]);

    rtGsmLarCoefficients13To26(previous, current, ratios);
    rtGsmLarToReflection(ratios);
    rtGsmShortTermAnalysisFiltering(
        codec, ratios, kShortTermSecondCount, &samples[kShortTermSecond]);

    rtGsmLarCoefficients27To39(previous, current, ratios);
    rtGsmLarToReflection(ratios);
    rtGsmShortTermAnalysisFiltering(codec, ratios, kShortTermThirdCount, &samples[kShortTermThird]);

    rtGsmLarCoefficients40To159(current, ratios);
    rtGsmLarToReflection(ratios);
    rtGsmShortTermAnalysisFiltering(
        codec, ratios, kShortTermFourthCount, &samples[kShortTermFourth]);
}

// NTSC-U/C: 0x002f2a48, PAL: 0x002fc9b8
static void rtGsmShortTermSynthesisFilter(rtSubbandCodec *codec,
                                          const int *logAreaRatios,
                                          const int *input,
                                          short *output) {
    int ratios[RT_SUBBAND_LAR_COUNT];
    int *current;
    int *previous;

    current = codec->logAreaRatios[codec->logAreaRatioRow];
    codec->logAreaRatioRow ^= 1;
    previous = codec->logAreaRatios[codec->logAreaRatioRow];

    rtGsmDecodeLogAreaRatios(logAreaRatios, current);

    rtGsmLarCoefficients0To12(previous, current, ratios);
    rtGsmLarToReflection(ratios);
    rtGsmShortTermSynthesisFiltering(
        codec, ratios, kShortTermFirstCount, &input[kShortTermFirst], &output[kShortTermFirst]);

    rtGsmLarCoefficients13To26(previous, current, ratios);
    rtGsmLarToReflection(ratios);
    rtGsmShortTermSynthesisFiltering(
        codec, ratios, kShortTermSecondCount, &input[kShortTermSecond], &output[kShortTermSecond]);

    rtGsmLarCoefficients27To39(previous, current, ratios);
    rtGsmLarToReflection(ratios);
    rtGsmShortTermSynthesisFiltering(
        codec, ratios, kShortTermThirdCount, &input[kShortTermThird], &output[kShortTermThird]);

    rtGsmLarCoefficients40To159(current, ratios);
    rtGsmLarToReflection(ratios);
    rtGsmShortTermSynthesisFiltering(
        codec, ratios, kShortTermFourthCount, &input[kShortTermFourth], &output[kShortTermFourth]);
}

void rtSubbandCoder(rtSubbandCodec *codec,
                    const short *samples,
                    int *logAreaRatios,
                    int *lags,
                    int *gains,
                    int *gridPositions,
                    int *amplitudes,
                    int *pulses) {
    int filtered[kFrameSamples];
    int *history;
    int *predicted;
    int subframe;
    int k;

    history = &codec->residual[kResidualHistory];
    predicted = history;
    rtGsmPreprocess(codec, samples, filtered);
    rtGsmLpcAnalysis(filtered, logAreaRatios);
    rtGsmShortTermAnalysisFilter(codec, logAreaRatios, filtered);

    for (subframe = 0; subframe < RT_SUBBAND_SUBFRAMES; ++subframe) {
        rtGsmLongTermPredictor(&filtered[subframe * kSubframeSamples],
                               history,
                               &g_anExcitation[kExcitationMargin],
                               predicted,
                               lags++,
                               gains++);
        rtGsmRpeEncode(&g_anExcitation[kExcitationMargin], amplitudes++, gridPositions++, pulses);
        for (k = 0; k < kSubframeSamples; ++k) {
            history[k] = g_anExcitation[kExcitationMargin + k] + predicted[k];
        }
        history += kSubframeSamples;
        predicted += kSubframeSamples;
        pulses += RT_SUBBAND_PULSES;
    }
    memcpy(codec->residual, &codec->residual[kFrameSamples], kResidualHistory * sizeof(int));
}

void rtSubbandDecoder(rtSubbandCodec *codec,
                      int *logAreaRatios,
                      int *lags,
                      int *gains,
                      int *gridPositions,
                      int *amplitudes,
                      int *pulses,
                      short *samples) {
    int excitation[kSubframeSamples];
    int filtered[kFrameSamples];
    int *history;
    int subframe;
    int k;

    history = &codec->residual[kResidualHistory];
    for (subframe = 0; subframe < RT_SUBBAND_SUBFRAMES; ++subframe) {
        rtGsmRpeDecode(*amplitudes, *gridPositions, pulses, excitation);
        rtGsmLongTermSynthesisFilter(codec, *lags, *gains, excitation, history);
        for (k = 0; k < kSubframeSamples; ++k) {
            filtered[(subframe * kSubframeSamples) + k] = history[k];
        }
        ++amplitudes;
        ++lags;
        ++gains;
        ++gridPositions;
        pulses += RT_SUBBAND_PULSES;
    }
    rtGsmShortTermSynthesisFilter(codec, logAreaRatios, filtered, samples);
    rtGsmPostprocess(codec, samples);
}

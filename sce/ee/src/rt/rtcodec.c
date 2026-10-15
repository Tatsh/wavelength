#include "rt/rtcodec.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "rt/rtlpc10.h"
#include "rt/rtmemory.h"

enum {
    // Linear prediction coder.
    kLpcFrameSamples = RT_LPC_FRAME_BYTES / 2,
    kLpcFrameBits = 320,
    kLpcRingSize = 600,
    kLpcRingClearSize = 360,
    kLpcSpeechBufferSize = 336,
    kLpcSynthBufferSize = 180,
    kLpcDefaultPitch = 60,
    kLpcMinPitch = 20,
    kLpcMaxPitch = 156,
    kLpcVoicedPitchCode = 4,
    kLpcVoicedTransition = 2,
    kLpcUnvoicedPitchCode = 0,
    kLpcTransitionPitchCode = 127,
    kLpcMaxRmsCode = 1023,
    kLpcRmsSearchStart = 32,
    kLpcRmsSearchStep = 16,
    kLpcRmsCodeBase = 31,
    kLpcLogAreaCount = 2,
    kLpcLogAreaShift = 9,
    kLpcLogAreaMaxIndex = 63,
    kLpcLogAreaMaxCode = 15,
    kLpcCodeLimit = 127,
    kLpcCodeShift = 15,
    kLpcParityMask = 30,
    kLpcSmoothedRcCount = 6,
    kLpcProtectedRcFirst = 4,
    kLpcErrorRateHigh = 2048,
    kLpcErrorRateMedium = 1024,
    kLpcErrorRateLow = 128,
    kLpcErrorWeight = 102,
    kLpcThresholdRows = 4,
    kLpcThresholdColumns = 8,
    kLpcVoicingTableSize = 32,
    kLpcPitchDecodeSize = 128,
    kLpcRcDecodeSize = 32,
    kLpcLinearRcCount = 8,
    kLpcCorrectionBitCount = 5,
    kLpcParityTableSize = 16,
    kLpcPitchEncodeSize = 60,
    kLpcLogAreaTableSize = 64,
    kLpcRmsTableSize = 64,
    kLpcHammingWords = 4,

    // Bits of the correction word of a voicing table entry.
    kLpcSmoothRc = 1,
    kLpcSmoothPitch = 2,
    kLpcCorrectParity = 3,
    kLpcClearHighRc = 4,

    // Pitch history adjustments of a voicing table entry.
    kLpcPitchFromNext = 1,
    kLpcPitchFromPrevious = 3,

    // Directions of rtLpc10Chanwr().
    kLpcPackWrite = 0,
    kLpcPackRead = 1,

    // Speech coder.
    kSpeechFrameSamples = RT_SPEECH_FRAME_BYTES / 2,
    kSpeechOrder = 10,
    kSpeechBufferLength = 240,
    kSpeechHistoryLength = kSpeechBufferLength - kSpeechFrameSamples,
    kSpeechMaxWindow = 1000,
    kSpeechDecimation = 5,
    kSpeechDecimatedLength = kSpeechBufferLength / kSpeechDecimation,
    kSpeechPitchOrder = 4,
    kSpeechMinLag = 5,
    kSpeechMaxLag = 32,
    kSpeechFilterTaps = 6,
    kSpeechMaxGain = 255,
    kSpeechVoiced = 3,
};

// The 14 bytes of one frame of the speech coder.
typedef struct {
    unsigned short period;
    unsigned char gain;
    unsigned char reserved; // +0x03
    signed char reflection[kSpeechOrder];
} rtSpeechFrame;

// The excitation and lattice of the speech synthesiser, retained from frame to frame.
typedef struct {
    double oldPeriod;
    double oldGain;
    double oldReflection[kSpeechOrder + 1];
    double lattice[kSpeechOrder + 1];
    int pitchCounter;
} rtSpeechSynthState;

// NTSC-U/C: 0x003b6f68, PAL: 0x00435e28
static int g_nAudioRingReadIdx;

// NTSC-U/C: 0x003b6f6c, PAL: 0x00435e2c
static int g_nAudioRingWriteIdx = kLpcRingClearSize;

// NTSC-U/C: 0x003b6f70, PAL: 0x00435e30
static unsigned int g_dwAudioRingInit = 1;

// NTSC-U/C: 0x003b6f78, the correction word of each voicing history.
static int g_anLpcVoicingTable[kLpcVoicingTableSize] = {
    24960, 24960, 24960, 24960, 25480, 25480, 25483, 25480, 16640, 1560,  1560,
    1560,  16640, 1816,  1563,  1560,  24960, 24960, 24859, 24856, 26001, 25881,
    25915, 25913, 1560,  1560,  7800,  3640,  1561,  1561,  3643,  3641,
};

// NTSC-U/C: 0x003b6ff8, PAL: 0x00435eb8, the smoothing thresholds by error rate and then by
// parameter.
static float g_aafLpcSmoothThreshold[kLpcThresholdRows][kLpcThresholdColumns] = {
    {32767.0f, 32767.0f, 32.0f, 32.0f, 32.0f, 32.0f, 16.0f, 16.0f},
    {10.0f, 8.0f, 6.3999996f, 6.3999996f, 11.2f, 11.2f, 5.6f, 5.6f},
    {5.0f, 4.0f, 3.1999998f, 3.1999998f, 6.3999996f, 6.3999996f, 3.1999998f, 3.1999998f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
};

// NTSC-U/C: 0x003b7078, the pitch or voicing of each pitch code.
static int g_anLpcPitchDecodeTable[kLpcPitchDecodeSize] = {
    0,   0,  0,   3,   0,   3,   3,   31, 0,  3,   3,   21,  3,  3,   29,  30, 0,  3,   3,
    20,  3,  25,  27,  26,  3,   23,  58, 22, 3,   24,  28,  3,  0,   3,   3,  3,  3,   39,
    33,  32, 3,   37,  35,  36,  3,   38, 34, 3,   3,   42,  46, 44,  50,  40, 48, 3,   54,
    3,   56, 3,   52,  3,   3,   1,   0,  3,  3,   108, 3,   78, 100, 104, 3,  84, 92,  88,
    156, 80, 96,  3,   3,   74,  70,  72, 66, 76,  68,  3,   62, 3,   60,  3,  64, 3,   3,
    1,   3,  116, 132, 112, 148, 152, 3,  3,  140, 3,   136, 3,  144, 3,   3,  1,  124, 120,
    128, 3,  3,   3,   3,   1,   3,   3,  3,  1,   3,   1,   1,  1,
};

// NTSC-U/C: 0x003b7278, PAL: 0x00436138, the magnitude of each log area ratio code.
static int g_anLpcRcDecodeTable[kLpcRcDecodeSize] = {
    4,   11,  18,  25,  32,  39,  46,  53,  60,  66,  72,  77,  82,  87,  92,  96,
    101, 104, 108, 111, 114, 115, 117, 119, 121, 122, 123, 124, 125, 126, 127, 127,
};

// NTSC-U/C: 0x003b72f8, PAL: 0x004361b8
static float g_afLpcRcDecodeScale[kLpcLinearRcCount] = {
    0.6953f,
    0.625f,
    0.57809997f,
    0.5469f,
    0.5312f,
    0.5391f,
    0.46879998f,
    0.38279998f,
};

// NTSC-U/C: 0x003b7318, PAL: 0x004361d8
static int g_anLpcRcDecodeBias[kLpcLinearRcCount] = {
    1152,
    -2816,
    -1536,
    -3584,
    -1280,
    -2432,
    768,
    -1920,
};

// NTSC-U/C: 0x003b7338, PAL: 0x004361f8
static int g_anLpcRcDecodeRound[kLpcLinearRcCount] = {
    511,
    511,
    1023,
    1023,
    1023,
    1023,
    2047,
    4095,
};

// NTSC-U/C: 0x003b7358, PAL: 0x00436218, the width of each reflection code.
static int g_anLpcRcBits[RT_LPC_ORDER] = {8, 8, 5, 5, 4, 4, 4, 4, 3, 2};

// NTSC-U/C: 0x003b7380, PAL: 0x00436240, the reflection codes an unvoiced frame decodes with.
static int g_anLpcRcUnvoiced[RT_LPC_ORDER] = {0, 0, 0, 0, 0, 3, 0, 2, 0, 0};

// NTSC-U/C: 0x003b73a8
static int g_anLpcCorrectionBits[kLpcCorrectionBitCount] = {2, 4, 8, 16, 32};

// NTSC-U/C: 0x003b73bc, PAL: 0x0043627c
static int g_nLpcPreviousHalfVoicing;

// NTSC-U/C: 0x003b73c0, PAL: 0x00436280
static int g_nLpcErrorRate;

// NTSC-U/C: 0x003b73c4, PAL: 0x00436284
static int g_nLpcPreviousVoicing;

// NTSC-U/C: 0x003b73c8, PAL: 0x00436288
static short g_wLpcFirstFrame = 1;

// NTSC-U/C: 0x003b73d0, PAL: 0x00436290, the parity word of each protected nibble.
static int g_anLpcParityTable[kLpcParityTableSize] = {
    0,
    7,
    11,
    12,
    13,
    10,
    6,
    1,
    14,
    9,
    5,
    2,
    3,
    4,
    8,
    15,
};

// NTSC-U/C: 0x003b7410, the pitch code of each voiced pitch.
static int g_anLpcPitchEncodeTable[kLpcPitchEncodeSize] = {
    19, 11, 27, 25, 29, 21, 23, 22, 30, 14, 15,  7,   39,  38, 46,  42,  43,  41,  45,  37,
    53, 49, 51, 50, 54, 52, 60, 56, 58, 26, 90,  88,  92,  84, 86,  82,  83,  81,  85,  69,
    77, 73, 75, 74, 78, 70, 71, 67, 99, 97, 113, 112, 114, 98, 106, 104, 108, 100, 101, 76,
};

// NTSC-U/C: 0x003b7500, PAL: 0x004363c0
static int g_anLpcRcEncodeBias[kLpcLinearRcCount] = {
    1920,
    -768,
    2432,
    1280,
    3584,
    1536,
    2816,
    -1152,
};

// NTSC-U/C: 0x003b7520, PAL: 0x004363e0
static float g_afLpcRcEncodeScale[kLpcLinearRcCount] = {
    0.020399999f,
    0.0167f,
    0.0145f,
    0.014699999f,
    0.0143f,
    0.013499999f,
    0.012499999f,
    0.0112f,
};

// NTSC-U/C: 0x003b7540, PAL: 0x00436400
static int g_anLpcRcEncodeBits[kLpcLinearRcCount] = {6, 5, 4, 4, 4, 4, 3, 3};

// NTSC-U/C: 0x003b7560, PAL: 0x00436420, the log area ratio code of each reflection magnitude.
static int g_anLpcLogAreaTable[kLpcLogAreaTableSize] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,  2,  2,  2,  2,  2,  2,  2,  3, 3,
    3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5,  5,  5,  5,  5,  6,  6,  6,  6, 6,
    7, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 13, 14, 15,
};

// NTSC-U/C: 0x003b7660, PAL: 0x00436520, the loudness of each loudness code, twice over.
static int g_anLpcRmsTable[kLpcRmsTableSize] = {
    1024, 936, 856, 784, 718, 656, 600, 550, 502, 460, 420, 384, 352, 328, 294, 270,
    246,  226, 206, 188, 172, 158, 144, 132, 120, 110, 102, 92,  84,  78,  70,  64,
    60,   54,  50,  46,  42,  38,  34,  32,  30,  26,  24,  22,  20,  18,  17,  16,
    15,   14,  13,  12,  11,  10,  9,   8,   7,   6,   5,   4,   3,   2,   1,   0,
};

// NTSC-U/C: 0x003b7760, PAL: 0x00436620, the voicing of the last two frames of the speech coder.
static int g_nVoicingState;

// NTSC-U/C: 0x00522d18
static float g_afAudioRing[kLpcRingSize];

// NTSC-U/C: 0x00523678
static int g_nLpcEncodePitch;

// NTSC-U/C: 0x0052367c
static float g_fLpcEncodeRms;

// NTSC-U/C: 0x00523680
static float g_afLpcEncodeRc[RT_LPC_ORDER];

// NTSC-U/C: 0x005236a8
static int g_anLpcEncodeVoice[2];

// NTSC-U/C: 0x005236b0
static int g_nFrameBitIndex;

// NTSC-U/C: 0x005236b4
static int g_nLpcEncodePitchCode;

// NTSC-U/C: 0x005236b8
static int g_nLpcEncodeRmsCode;

// NTSC-U/C: 0x005236c0
static int g_anLpcEncodeRcCode[RT_LPC_ORDER];

// NTSC-U/C: 0x005236e8
static int g_anLpcEncodeBits[kLpcFrameBits];

// NTSC-U/C: 0x00523be8
static float g_afLpcEncodeSpeech[kLpcSpeechBufferSize];

// NTSC-U/C: 0x00524128
static int g_nLpcDecodePitch;

// NTSC-U/C: 0x0052412c
static float g_fLpcDecodeRms;

// NTSC-U/C: 0x00524130
static float g_afLpcDecodeRc[RT_LPC_ORDER];

// NTSC-U/C: 0x00524158
static int g_anLpcDecodeVoice[2];

// NTSC-U/C: 0x00524160
static int g_nDecodeBitIndex;

// NTSC-U/C: 0x00524164
static int g_nLpcDecodePitchCode;

// NTSC-U/C: 0x00524168
static int g_nLpcDecodeRmsCode;

// NTSC-U/C: 0x00524170
static int g_anLpcDecodeRcCode[RT_LPC_ORDER];

// NTSC-U/C: 0x00524198
static int g_anLpcDecodeBits[kLpcFrameBits];

// NTSC-U/C: 0x00524698
static float g_afLpcDecodeSpeech[kLpcSpeechBufferSize];

// NTSC-U/C: 0x00524bd8
static float g_afLpcSynthBuffer[kLpcSynthBufferSize];

// NTSC-U/C: 0x00524ea8
static float g_afSpeechInput[kSpeechMaxWindow];

// NTSC-U/C: 0x00525e48
static float g_afSpeechFiltered[kSpeechMaxWindow];

// NTSC-U/C: 0x00526de8
static float g_afSpeechWindow[kSpeechMaxWindow];

// NTSC-U/C: 0x00527d88
static float g_afSpeechWindowed[kSpeechMaxWindow];

// NTSC-U/C: 0x00528d28
static rtSpeechSynthState g_SpeechSynthState;

// NTSC-U/C: 0x00528df0, the coefficients of the input filter, from one.
static float g_afSpeechFilter[kSpeechFilterTaps];

// NTSC-U/C: 0x00528e08
static float g_fSpeechFilterInput;

// NTSC-U/C: 0x00528e0c
static float g_fSpeechFilterInputPrevious;

// NTSC-U/C: 0x00528e10
static float g_fSpeechFilterOutputPrevious;

// NTSC-U/C: 0x00528e14
static float g_fSpeechFilterOutputPrevious2;

// NTSC-U/C: 0x00549218
int g_aanLpcRcHistory[RT_LPC_HISTORY][RT_LPC_ORDER];

// NTSC-U/C: 0x00549290
int g_anLpcPitchHistory[RT_LPC_HISTORY];

// NTSC-U/C: 0x005492a0
int g_anLpcRmsHistory[RT_LPC_HISTORY];

// NTSC-U/C: 0x005495c0
int g_anLpcEpochPitch[RT_LPC_MAX_EPOCHS];

// NTSC-U/C: 0x005495f0
int g_anLpcEpochVoicing[RT_LPC_MAX_EPOCHS];

// NTSC-U/C: 0x00549620
float g_aafLpcEpochRc[RT_LPC_ORDER][RT_LPC_MAX_EPOCHS];

// NTSC-U/C: 0x005497d8
float g_afLpcEpochRms[RT_LPC_MAX_EPOCHS];

// NTSC-U/C: 0x00549808
float g_afLpcPredictor[RT_LPC_ORDER];

// NTSC-U/C: 0x002eb028, PAL: 0x002f4f98
static void rtLpcRingExchange(const float *input, float *output, int count) {
    int i;

    if (g_dwAudioRingInit != 0) {
        g_dwAudioRingInit = 0;
        for (i = 0; i < kLpcRingClearSize; ++i) {
            g_afAudioRing[i] = 0.0f;
        }
    }
    for (i = 0; i < count; ++i) {
        g_afAudioRing[g_nAudioRingWriteIdx] = input[i];
        ++g_nAudioRingWriteIdx;
        if (g_nAudioRingWriteIdx == kLpcRingSize) {
            g_nAudioRingWriteIdx = 0;
        }
    }
    for (i = 0; i < kLpcFrameSamples; ++i) {
        output[i] = g_afAudioRing[g_nAudioRingReadIdx];
        ++g_nAudioRingReadIdx;
        if (g_nAudioRingReadIdx == kLpcRingSize) {
            g_nAudioRingReadIdx = 0;
        }
    }
}

void rtLpcReset(void) {
    rtLpc10ResetState();
    rtLpc10ResetAnalysis();
}

// NTSC-U/C: 0x002ebfe4, PAL: 0x002f5f54
static void rtLpcQuantize(const int *voice,
                          const int *pitch,
                          const float *rms,
                          const float *rc,
                          int *pitchCode,
                          int *rmsCode,
                          int *rcCode) {
    int i;
    int j;
    int step;
    int rmsValue;
    int code;
    int negative;
    int bias;
    int bits;

    *rmsCode = (int)*rms;
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        rcCode[i] = (int)(rc[i] * 32768.0f);
    }

    if (voice[0] != 0 && voice[1] != 0) {
        *pitchCode = g_anLpcPitchEncodeTable[*pitch - 1];
    } else {
        *pitchCode = kLpcUnvoicedPitchCode;
    }
    if (voice[0] != voice[1]) {
        *pitchCode = kLpcTransitionPitchCode;
    }

    j = kLpcRmsSearchStart;
    step = kLpcRmsSearchStep;
    if (*rmsCode > kLpcMaxRmsCode) {
        *rmsCode = kLpcMaxRmsCode;
    }
    rmsValue = *rmsCode;
    do {
        if (rmsValue > g_anLpcRmsTable[j - 1]) {
            j -= step;
        }
        if (rmsValue < g_anLpcRmsTable[j - 1]) {
            j += step;
        }
        step = (int)(step * 0.5);
    } while (step > 0);
    if (*rmsCode > g_anLpcRmsTable[j - 1]) {
        --j;
    }
    *rmsCode = (int)(kLpcRmsCodeBase - j * 0.5);

    for (i = 0; i < kLpcLogAreaCount; ++i) {
        code = rcCode[i];
        negative = 0;
        if (code < 0) {
            code = -code;
            negative = 1;
        }
        code >>= kLpcLogAreaShift;
        if (code > kLpcLogAreaMaxIndex) {
            code = kLpcLogAreaMaxIndex;
        }
        code = g_anLpcLogAreaTable[code];
        if (negative != 0) {
            code = -code;
        }
        rcCode[i] = code;
    }

    for (i = kLpcLogAreaCount; i < RT_LPC_ORDER; ++i) {
        j = RT_LPC_ORDER - 1 - i;
        code = rcCode[i] >> 1;
        bias = g_anLpcRcEncodeBias[j];
        if (bias < 0) {
            code = -(int)((float)-(code + bias) * g_afLpcRcEncodeScale[j]);
        } else {
            code = (int)((float)(code + bias) * g_afLpcRcEncodeScale[j]);
        }
        if (code > kLpcCodeLimit) {
            code = kLpcCodeLimit;
        } else if (code < -kLpcCodeLimit) {
            code = -kLpcCodeLimit;
        }
        bits = g_anLpcRcEncodeBits[j];
        negative = code < 0;
        code /= 2 << (bits - 1);
        if (negative) {
            --code;
        }
        rcCode[i] = code;
    }

    if (*pitchCode == kLpcUnvoicedPitchCode || *pitchCode == kLpcTransitionPitchCode) {
        rcCode[4] = g_anLpcParityTable[(rcCode[0] & kLpcParityMask) / 2];
        rcCode[5] = g_anLpcParityTable[(rcCode[1] & kLpcParityMask) / 2];
        rcCode[6] = g_anLpcParityTable[(rcCode[2] & kLpcParityMask) / 2];
        rcCode[7] = g_anLpcParityTable[(*rmsCode & kLpcParityMask) / 2];
        rcCode[8] = g_anLpcParityTable[(rcCode[3] & kLpcParityMask) / 2] >> 1;
        rcCode[9] = g_anLpcParityTable[(rcCode[3] & kLpcParityMask) / 2] & 1;
    }
}

int rtLpcEncode(const short *samples, unsigned char *packed) {
    int i;

    for (i = 0; i < kLpcFrameSamples; ++i) {
        g_afLpcEncodeSpeech[i] = (float)samples[i] * 0x1p-15f;
    }
    rtLpc10Hp100(g_afLpcEncodeSpeech);
    rtLpc10Analys(g_afLpcEncodeSpeech,
                  g_anLpcEncodeVoice,
                  &g_nLpcEncodePitch,
                  &g_fLpcEncodeRms,
                  g_afLpcEncodeRc);
    memset(g_anLpcEncodeBits, 0, sizeof(g_anLpcEncodeBits));
    memset(packed, 0, RT_LPC_PACKED_SIZE);
    rtLpcQuantize(g_anLpcEncodeVoice,
                  &g_nLpcEncodePitch,
                  &g_fLpcEncodeRms,
                  g_afLpcEncodeRc,
                  &g_nLpcEncodePitchCode,
                  &g_nLpcEncodeRmsCode,
                  g_anLpcEncodeRcCode);
    rtLpc10Chanwr(kLpcPackWrite,
                  &g_nLpcEncodePitchCode,
                  &g_nLpcEncodeRmsCode,
                  g_anLpcEncodeRcCode,
                  g_anLpcEncodeBits);
    // Yes, the binary rewrites all 40 bytes the 320 bits cover, past the 7 bytes of the frame.
    for (g_nFrameBitIndex = 0; g_nFrameBitIndex < kLpcFrameBits; ++g_nFrameBitIndex) {
        packed[g_nFrameBitIndex >> 3] |=
            (g_anLpcEncodeBits[g_nFrameBitIndex] != 0) ? (1 << (g_nFrameBitIndex & 7)) : 0;
    }
    return RT_LPC_PACKED_SIZE;
}

// NTSC-U/C: 0x002ec3c4, PAL: 0x002f6334
static int rtLpcMedianOfThree(int first, int second, int third) {
    int median = second;

    if (second > first && second > third) {
        median = first;
        if (third > median) {
            median = third;
        }
    } else if (second < first && second < third) {
        median = first;
        if (third < median) {
            median = third;
        }
    }
    return median;
}

// NTSC-U/C: 0x002eb504, PAL: 0x002f5474
static void rtLpcDequantize(
    int pitchCode, int *rmsCode, int *rcCode, int *voice, int *pitch, float *rms, float *rc) {
    int i;
    int word;
    int code;
    int decodedPitch;
    int pitchAdjust;
    int correction;
    int voicing;
    int threshold;
    int index;
    int corrected;
    int errors;
    int lsb;
    int shift;
    int negative;
    float scaled;

    decodedPitch = g_anLpcPitchDecodeTable[pitchCode];
    if (decodedPitch > kLpcVoicedPitchCode) {
        g_anLpcPitchHistory[0] = decodedPitch;
        voicing = kLpcVoicedTransition;
    } else {
        voicing = decodedPitch;
        g_anLpcPitchHistory[0] = kLpcDefaultPitch;
    }
    g_anLpcRmsHistory[0] = *rmsCode;
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        g_aanLpcRcHistory[0][i] = rcCode[i];
    }

    word = g_anLpcVoicingTable[((g_nLpcPreviousHalfVoicing * 4) + g_nLpcPreviousVoicing) * 4 +
                               voicing];
    pitchAdjust = word & 3;
    correction = (int)(word * 0.125);
    if (g_nLpcErrorRate < kLpcErrorRateHigh) {
        correction = (int)(correction * 0.015625);
    }
    threshold = 4;
    if (g_nLpcErrorRate < kLpcErrorRateHigh) {
        threshold = 3;
    }
    if (g_nLpcErrorRate < kLpcErrorRateMedium) {
        threshold = 2;
    }
    if (g_nLpcErrorRate < kLpcErrorRateLow) {
        threshold = 1;
    }
    voice[0] = (int)(correction * 0.5) & 1;
    voice[1] = correction & 1;

    if (g_wLpcFirstFrame != 0) {
        // Yes, the binary does not set the pitch on the first frame.
        g_wLpcFirstFrame = 0;
    } else {
        if ((correction & g_anLpcCorrectionBits[kLpcCorrectParity]) != 0) {
            // Yes, the binary passes the count to the Hamming decoder, which never updates it.
            errors = 0;
            lsb = g_anLpcRmsHistory[1] & 1;
            index = (int)((g_aanLpcRcHistory[1][7] << 4) + g_anLpcRmsHistory[1] * 0.5);
            rtLpc10Ham84(index, &corrected);
            g_anLpcRmsHistory[1] = g_anLpcRmsHistory[2];
            if (corrected >= 0) {
                g_anLpcRmsHistory[1] = (corrected << 1) + lsb;
            }
            for (i = 1; i <= kLpcHammingWords; ++i) {
                if (i == 1) {
                    code = ((g_aanLpcRcHistory[1][8] & 7) << 1) + (g_aanLpcRcHistory[1][9] & 1);
                } else {
                    code = g_aanLpcRcHistory[1][8 - i] & 15;
                }
                lsb = g_aanLpcRcHistory[1][4 - i] & 1;
                index = (int)((code << 4) + (g_aanLpcRcHistory[1][4 - i] & 31) * 0.5);
                rtLpc10Ham84(index, &corrected);
                if (corrected >= 0) {
                    corrected = (corrected << 1) + lsb;
                    if ((corrected & 16) != 0) {
                        corrected -= 32;
                    }
                } else {
                    corrected = g_aanLpcRcHistory[2][4 - i];
                }
                g_aanLpcRcHistory[1][4 - i] = corrected;
            }
            g_nLpcErrorRate = (int)(g_nLpcErrorRate * 0.96875 + errors * kLpcErrorWeight);
        }

        *rmsCode = g_anLpcRmsHistory[1];
        for (i = 0; i < RT_LPC_ORDER; ++i) {
            rcCode[i] = g_aanLpcRcHistory[1][i];
        }
        if (pitchAdjust == kLpcPitchFromNext) {
            g_anLpcPitchHistory[1] = g_anLpcPitchHistory[2];
        }
        if (pitchAdjust == kLpcPitchFromPrevious) {
            g_anLpcPitchHistory[1] = g_anLpcPitchHistory[0];
        }
        *pitch = g_anLpcPitchHistory[1];

        if ((correction & g_anLpcCorrectionBits[kLpcSmoothRc]) != 0) {
            if ((float)abs(g_anLpcRmsHistory[1] - g_anLpcRmsHistory[0]) >=
                    g_aafLpcSmoothThreshold[threshold - 1][1] &&
                (float)abs(g_anLpcRmsHistory[1] - g_anLpcRmsHistory[2]) >=
                    g_aafLpcSmoothThreshold[threshold - 1][1]) {
                *rmsCode = rtLpcMedianOfThree(
                    g_anLpcRmsHistory[2], g_anLpcRmsHistory[1], g_anLpcRmsHistory[0]);
            }
            for (i = 0; i < kLpcSmoothedRcCount; ++i) {
                if ((float)abs(g_aanLpcRcHistory[1][i] - g_aanLpcRcHistory[0][i]) >=
                        g_aafLpcSmoothThreshold[threshold - 1][i + 2] &&
                    (float)abs(g_aanLpcRcHistory[1][i] - g_aanLpcRcHistory[2][i]) >=
                        g_aafLpcSmoothThreshold[threshold - 1][i + 2]) {
                    rcCode[i] = rtLpcMedianOfThree(
                        g_aanLpcRcHistory[2][i], g_aanLpcRcHistory[1][i], g_aanLpcRcHistory[0][i]);
                }
            }
        }

        if ((correction & g_anLpcCorrectionBits[kLpcSmoothPitch]) != 0) {
            if ((float)abs(g_anLpcPitchHistory[1] - g_anLpcPitchHistory[0]) >=
                    g_aafLpcSmoothThreshold[threshold - 1][0] &&
                (float)abs(g_anLpcPitchHistory[1] - g_anLpcPitchHistory[2]) >=
                    g_aafLpcSmoothThreshold[threshold - 1][0]) {
                *pitch = rtLpcMedianOfThree(
                    g_anLpcPitchHistory[2], g_anLpcPitchHistory[1], g_anLpcPitchHistory[0]);
            }
        }
    }

    if ((correction & g_anLpcCorrectionBits[kLpcClearHighRc]) != 0) {
        for (i = kLpcProtectedRcFirst; i < RT_LPC_ORDER; ++i) {
            rcCode[i] = g_anLpcRcUnvoiced[i];
        }
    }

    g_nLpcPreviousVoicing = voicing;
    g_nLpcPreviousHalfVoicing = voice[1];
    g_anLpcPitchHistory[2] = g_anLpcPitchHistory[1];
    g_anLpcPitchHistory[1] = g_anLpcPitchHistory[0];
    g_anLpcRmsHistory[2] = g_anLpcRmsHistory[1];
    g_anLpcRmsHistory[1] = g_anLpcRmsHistory[0];
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        g_aanLpcRcHistory[2][i] = g_aanLpcRcHistory[1][i];
        g_aanLpcRcHistory[1][i] = g_aanLpcRcHistory[0][i];
    }

    *rmsCode = g_anLpcRmsTable[(kLpcRmsCodeBase - *rmsCode) * 2];

    for (i = 0; i < kLpcLogAreaCount; ++i) {
        code = rcCode[i];
        negative = 0;
        if (code < 0) {
            negative = 1;
            code = -code;
            if (code > kLpcLogAreaMaxCode) {
                code = 0;
            }
        }
        code = g_anLpcRcDecodeTable[code * 2];
        if (negative == 1) {
            code = -code;
        }
        shift = kLpcCodeShift - g_anLpcRcBits[i];
        rcCode[i] = code * (2 << (shift - 1));
    }

    for (i = kLpcLogAreaCount; i < RT_LPC_ORDER; ++i) {
        code = rcCode[i];
        shift = kLpcCodeShift - g_anLpcRcBits[i];
        code *= 2 << (shift - 1);
        code += g_anLpcRcDecodeRound[i - kLpcLogAreaCount];
        scaled = (float)code * g_afLpcRcDecodeScale[i - kLpcLogAreaCount] +
                 (float)g_anLpcRcDecodeBias[i - kLpcLogAreaCount];
        if (scaled < 0.0f) {
            rcCode[i] = -(int)-scaled;
        } else {
            rcCode[i] = (int)scaled;
        }
    }

    *rms = (float)*rmsCode;
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        rc[i] = (float)(rcCode[i] * 6.103515625e-05);
    }
}

// NTSC-U/C: 0x002ec424, PAL: 0x002f6394
static void
rtLpcSynthesizeFrame(int *voice, int *pitch, float *rms, float *rc, float *speech, int *count) {
    int i;
    int j;
    int epochs;
    float ratio;
    float gain;
    float clamped;

    if (*pitch <= kLpcMinPitch) {
        *pitch = kLpcMinPitch;
    } else if (*pitch > kLpcMaxPitch) {
        *pitch = kLpcMaxPitch;
    }
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        clamped = (rc[i] < 0.99f) ? rc[i] : 0.99f;
        rc[i] = (clamped > -0.99f) ? clamped : -0.99f;
    }
    *count = 0;
    rtLpc10Pitsyn(voice,
                  pitch,
                  rms,
                  rc,
                  g_anLpcEpochVoicing,
                  g_anLpcEpochPitch,
                  g_afLpcEpochRms,
                  g_aafLpcEpochRc,
                  &epochs,
                  &ratio);
    if (epochs > 0) {
        for (j = 0; j < epochs; ++j) {
            rtLpc10Irc2pc(g_aafLpcEpochRc, g_afLpcPredictor, &gain, j, 0.7f);
            rtLpc10Bsynz(g_afLpcPredictor,
                         g_anLpcEpochPitch[j],
                         g_anLpcEpochVoicing[j],
                         g_afLpcSynthBuffer,
                         g_afLpcEpochRms[j],
                         ratio,
                         gain);
            rtLpc10Deemp(g_afLpcSynthBuffer, g_anLpcEpochPitch[j]);
            for (i = 0; i < g_anLpcEpochPitch[j]; ++i) {
                ++*count;
                speech[*count - 1] = g_afLpcSynthBuffer[i] * 0x1p-12f;
            }
        }
    }
}

int rtLpcDecode(const unsigned char *packed, short *samples) {
    int i;
    int count;
    double sample;

    // Yes, the binary reads all 40 bytes the 320 bits cover, past the 7 bytes of the frame.
    for (g_nDecodeBitIndex = 0; g_nDecodeBitIndex < kLpcFrameBits; ++g_nDecodeBitIndex) {
        g_anLpcDecodeBits[g_nDecodeBitIndex] =
            (packed[g_nDecodeBitIndex >> 3] >> (g_nDecodeBitIndex & 7)) & 1;
    }
    rtLpc10Chanwr(kLpcPackRead,
                  &g_nLpcDecodePitchCode,
                  &g_nLpcDecodeRmsCode,
                  g_anLpcDecodeRcCode,
                  g_anLpcDecodeBits);
    rtLpcDequantize(g_nLpcDecodePitchCode,
                    &g_nLpcDecodeRmsCode,
                    g_anLpcDecodeRcCode,
                    g_anLpcDecodeVoice,
                    &g_nLpcDecodePitch,
                    &g_fLpcDecodeRms,
                    g_afLpcDecodeRc);
    rtLpcSynthesizeFrame(g_anLpcDecodeVoice,
                         &g_nLpcDecodePitch,
                         &g_fLpcDecodeRms,
                         g_afLpcDecodeRc,
                         g_afLpcDecodeSpeech,
                         &count);
    rtLpcRingExchange(g_afLpcDecodeSpeech, g_afLpcDecodeSpeech, count);
    for (i = 0; i < kLpcFrameSamples; ++i) {
        sample = g_afLpcDecodeSpeech[i] * 32768.0;
        if (sample < 32767.0 && sample < -32768.0) {
            samples[i] = -32768;
        } else if (sample >= 32767.0) {
            samples[i] = 32767;
        } else {
            samples[i] = (short)(int)sample;
        }
    }
    return kLpcFrameSamples;
}

// NTSC-U/C: 0x002ec6a0, PAL: 0x002f6610
static void rtSpeechAutocorrelate(const float *input, int length, int order, float *correlation) {
    int i;
    int lag;
    int count;

    for (lag = 0; lag <= order; ++lag) {
        count = length - lag;
        correlation[lag] = 0.0f;
        for (i = 0; i < count; ++i) {
            correlation[lag] += input[i] * input[i + lag];
        }
    }
}

// NTSC-U/C: 0x002ec718, PAL: 0x002f6688
static void
rtSpeechLevinsonDurbin(const float *correlation, int order, float *reflection, float *gain) {
    int i;
    int j;
    float predictor[kSpeechOrder + 1];
    float previous[kSpeechOrder + 1];
    float error;

    for (i = 0; i <= order; ++i) {
        predictor[i] = previous[i] = 0.0f;
    }
    error = correlation[0];
    for (i = 1; i <= order; ++i) {
        reflection[i] = -correlation[i];
        for (j = 1; j < i; ++j) {
            previous[j] = predictor[j];
            reflection[i] -= predictor[j] * correlation[i - j];
        }
        if (error == 0.0f) {
            error = 0.0f;
            break;
        }
        reflection[i] /= error;
        predictor[i] = reflection[i];
        for (j = 1; j < i; ++j) {
            predictor[j] = previous[j] + reflection[i] * previous[i - j];
        }
        error *= 1.0f - reflection[i] * reflection[i];
    }
    if (error < 0.0f) {
        error = 0.0f;
    }
    *gain = (float)sqrt(error);
}

// NTSC-U/C: 0x002ec8d0, PAL: 0x002f6840
static void rtSpeechLatticeFilter(float *signal, const float *reflection) {
    int i;
    int j;
    float backward[kSpeechPitchOrder + 1];
    float backwardPrevious[kSpeechPitchOrder + 1];
    float forward[kSpeechPitchOrder + 1];

    for (i = 0; i <= kSpeechPitchOrder; ++i) {
        backward[i] = forward[i] = backwardPrevious[i] = 0.0f;
    }
    for (i = 0; i < kSpeechDecimatedLength; ++i) {
        forward[0] = backward[0] = signal[i];
        for (j = 1; j <= kSpeechPitchOrder; ++j) {
            forward[j] = forward[j - 1] + reflection[j] * backwardPrevious[j - 1];
            backward[j] = reflection[j] * forward[j - 1] + backwardPrevious[j - 1];
            backwardPrevious[j - 1] = backward[j - 1];
        }
        signal[i] = forward[kSpeechPitchOrder];
    }
}

// NTSC-U/C: 0x002ec9c0, PAL: 0x002f6930
static void rtSpeechEstimatePitch(const float *signal, float *period) {
    int i;
    int j;
    int n;
    int peak;
    float decimated[kSpeechMaxWindow / kSpeechDecimation];
    float reflection[kSpeechPitchOrder + 1];
    float correlation[kSpeechMaxLag + 2];
    float gain;
    float peakValue;
    float before;
    float after;
    float a;
    float b;
    float c;
    float x;
    float ratio;

    for (i = 0, j = 0; i < kSpeechBufferLength; ++j) {
        decimated[j] = 0.0f;
        for (n = 0; n < kSpeechDecimation; ++n) {
            decimated[j] += signal[i];
            ++i;
        }
    }
    rtSpeechAutocorrelate(decimated, kSpeechDecimatedLength, kSpeechPitchOrder, correlation);
    rtSpeechLevinsonDurbin(correlation, kSpeechPitchOrder, reflection, &gain);
    rtSpeechLatticeFilter(decimated, reflection);
    rtSpeechAutocorrelate(decimated, kSpeechDecimatedLength, kSpeechMaxLag, correlation);

    peak = 0;
    peakValue = 0.0f;
    for (i = kSpeechMinLag; i <= kSpeechMaxLag; ++i) {
        if (correlation[i] > peakValue) {
            peakValue = correlation[i];
            peak = i;
        }
    }

    // Yes, the binary reads before the correlation when no lag peaks.
    before = correlation[peak - 1];
    after = correlation[peak + 1];
    a = 0.5f * before - peakValue + 0.5f * after;
    b = -0.5f * before * (2.0f * peak + 1.0f) + 2.0f * peak * peakValue +
        0.5f * after * (1.0f - 2.0f * peak);
    c = 0.5f * before * (peak * peak + peak) + peakValue * (1.0f - peak * peak) +
        0.5f * after * (peak * peak - peak);
    x = -b / (2.0f * a);
    peakValue = a * x * x + b * x + c;
    x *= 5.0f;

    if (correlation[0] == 0.0) {
        ratio = 1.0f;
    } else {
        ratio = peakValue / correlation[0];
    }
    if ((ratio >= 0x1.9999999999999p-2 || (g_nVoicingState == kSpeechVoiced && ratio >= 0.3)) &&
        x > 0.0f) {
        *period = x;
        g_nVoicingState = (g_nVoicingState & 1) * 2 + 1;
    } else {
        *period = 0.0f;
        g_nVoicingState = (g_nVoicingState & 1) * 2;
    }
}

void rtSpeechInit(void) {
    int i;

    for (i = 0; i < kSpeechBufferLength; ++i) {
        g_afSpeechInput[i] = 0.0f;
        g_afSpeechWindow[i] = (float)((0x1.147ae147ae147p-1 -
                                       0x1.d70a3d70a3d70p-2 * cos(i * 6.283185307179586 /
                                                                  (kSpeechBufferLength - 1.0))) *
                                      0x1.9617c1bda5119p+0);
    }
    g_afSpeechFilter[1] = (float)(0.0 - exp(-0x1.6408e6p-3));
    g_afSpeechFilter[2] = g_afSpeechFilter[1] + 1.0f;
    g_afSpeechFilter[3] = (float)(exp(-0x1.6408e6p-4) * -2.0 * cos(0x1.bd6df8p-2));
    g_afSpeechFilter[4] = (float)exp(-0x1.6408e6p-3);
    g_afSpeechFilter[5] = g_afSpeechFilter[3] + 1.0f + g_afSpeechFilter[4];
    g_fSpeechFilterInputPrevious = 0.0f;
    g_fSpeechFilterOutputPrevious = 0.0f;
    g_fSpeechFilterOutputPrevious2 = 0.0f;
    g_SpeechSynthState.oldPeriod = 0.0;
    g_SpeechSynthState.oldGain = 0.0;
    for (i = 0; i <= kSpeechOrder; ++i) {
        g_SpeechSynthState.oldReflection[i] = 0.0;
        g_SpeechSynthState.lattice[i] = 0.0;
    }
    g_SpeechSynthState.pitchCounter = 0;
}

int rtSpeechEncode(const short *samples, unsigned char *packed) {
    int i;
    int j;
    int level;
    float correlation[kSpeechOrder + 1];
    float reflection[kSpeechOrder + 1];
    float period;
    float gain;
    rtSpeechFrame frame;

    for (i = 0, j = kSpeechHistoryLength; i < kSpeechFrameSamples; ++i, ++j) {
        g_afSpeechInput[j] = (float)(samples[i] * 0x1p-15 * 0x1.cccccccccccccp-1);
        g_fSpeechFilterInput = g_afSpeechFilter[2] * g_afSpeechInput[j] -
                               g_afSpeechFilter[1] * g_fSpeechFilterInputPrevious;
        g_afSpeechFiltered[j] = g_afSpeechFilter[5] * g_fSpeechFilterInputPrevious -
                                g_afSpeechFilter[3] * g_fSpeechFilterOutputPrevious -
                                g_afSpeechFilter[4] * g_fSpeechFilterOutputPrevious2;
        g_fSpeechFilterInputPrevious = g_fSpeechFilterInput;
        g_fSpeechFilterOutputPrevious2 = g_fSpeechFilterOutputPrevious;
        g_fSpeechFilterOutputPrevious = g_afSpeechFiltered[j];
    }

    rtSpeechEstimatePitch(g_afSpeechFiltered, &period);
    for (i = 0; i < kSpeechBufferLength; ++i) {
        g_afSpeechWindowed[i] = g_afSpeechInput[i] * g_afSpeechWindow[i];
    }
    rtSpeechAutocorrelate(g_afSpeechWindowed, kSpeechBufferLength, kSpeechOrder, correlation);
    rtSpeechLevinsonDurbin(correlation, kSpeechOrder, reflection, &gain);

    frame.period = (unsigned short)(unsigned int)(period * 256.0f);
    level = (int)(gain * 256.0f);
    if (level > kSpeechMaxGain) {
        level = kSpeechMaxGain;
    }
    frame.gain = (unsigned char)level;
    for (i = 0; i < kSpeechOrder; ++i) {
        // Yes, the binary compares the coefficient with -0.9999 and 0.9999 and discards both.
        frame.reflection[i] = (signed char)(int)(reflection[i + 1] * 127.0 + 0.5);
    }

    memmove(g_afSpeechInput,
            &g_afSpeechInput[kSpeechFrameSamples],
            kSpeechHistoryLength * sizeof(float));
    memmove(g_afSpeechFiltered,
            &g_afSpeechFiltered[kSpeechFrameSamples],
            kSpeechHistoryLength * sizeof(float));
    // Yes, the binary copies the unset fourth byte of the frame.
    memcpy(packed, &frame, sizeof(frame));
    return RT_SPEECH_PACKED_SIZE;
}

int rtSpeechDecode(const unsigned char *packed, short *samples) {
    int i;
    int j;
    rtSpeechFrame frame;
    double period;
    double gain;
    double reflection[kSpeechOrder + 1];
    double newReflection[kSpeechOrder + 1];
    double reflectionStep[kSpeechOrder + 1];
    double newPeriod;
    double newGain;
    double periodStep;
    double gainStep;
    double excitation;
    double value;

    memcpy(&frame, packed, sizeof(frame));
    period = frame.period * 0.00390625;
    gain = frame.gain * 0.00390625;
    reflection[0] = 0.0;
    for (i = 0; i < kSpeechOrder; ++i) {
        reflection[i + 1] = frame.reflection[i] * 0.0078125;
    }
    gain /= sqrt((period != 0.0) ? (kSpeechBufferLength / period) : 80.0);

    newPeriod = g_SpeechSynthState.oldPeriod;
    newGain = g_SpeechSynthState.oldGain;
    for (i = 1; i <= kSpeechOrder; ++i) {
        newReflection[i] = g_SpeechSynthState.oldReflection[i];
    }
    if (g_SpeechSynthState.oldPeriod != 0.0 && period != 0.0) {
        periodStep = (period - g_SpeechSynthState.oldPeriod) / kSpeechFrameSamples;
        gainStep = (gain - g_SpeechSynthState.oldGain) / kSpeechFrameSamples;
        for (i = 1; i <= kSpeechOrder; ++i) {
            reflectionStep[i] =
                (reflection[i] - g_SpeechSynthState.oldReflection[i]) / kSpeechFrameSamples;
        }
    } else {
        gainStep = 0.0;
        periodStep = 0.0;
        for (i = 1; i <= kSpeechOrder; ++i) {
            reflectionStep[i] = 0.0;
        }
    }
    if (newPeriod == 0.0) {
        g_SpeechSynthState.pitchCounter = 0;
    }

    for (i = 0; i < kSpeechFrameSamples; ++i) {
        if (newPeriod == 0.0) {
            excitation = ((double)rand() * 0x1p-31 - 0.5) * 1.5874 * newGain;
        } else if (g_SpeechSynthState.pitchCounter == 0) {
            excitation = newGain;
            g_SpeechSynthState.pitchCounter = (int)newPeriod;
        } else {
            excitation = 0.0;
            --g_SpeechSynthState.pitchCounter;
        }

        value = excitation;
        for (j = kSpeechOrder; j >= 1; --j) {
            value -= g_SpeechSynthState.lattice[j - 1] * newReflection[j];
            g_SpeechSynthState.lattice[j] =
                g_SpeechSynthState.lattice[j - 1] + value * newReflection[j];
            newReflection[j] += reflectionStep[j];
        }
        g_SpeechSynthState.lattice[0] = value;
        if (value < -0x1.fff2e48e8a71dp-1) {
            value = -0x1.fff2e48e8a71dp-1;
        } else if (value > 0x1.fff2e48e8a71dp-1) {
            value = 0x1.fff2e48e8a71dp-1;
        }
        samples[i] = (short)(int)(value * 32767.0);
        newPeriod += periodStep;
        newGain += gainStep;
    }

    g_SpeechSynthState.oldPeriod = period;
    g_SpeechSynthState.oldGain = gain;
    for (i = 1; i <= kSpeechOrder; ++i) {
        g_SpeechSynthState.oldReflection[i] = reflection[i];
    }
    return kSpeechFrameSamples;
}

// NTSC-U/C: 0x002ed70c, PAL: 0x002f767c
static void rtSpeechCodecFreeBuffer(rtSpeechCodec *codec) {
    if (codec->buffer != NULL) {
        RtFreeBlock(&codec->buffer);
        codec->buffer = NULL;
        codec->bufferSize = 0;
    }
}

void rtSpeechDestroy(rtSpeechCodec *codec) {
    void *block = codec;

    if (codec != NULL) {
        rtSpeechCodecFreeBuffer(codec);
        RtFreeBlock(&block);
    }
}

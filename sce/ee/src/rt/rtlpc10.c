#include "rt/rtlpc10.h"

#include <math.h>
#include <string.h>

#include "rt/rtpitch.h"

enum {
    kLpcFrame = 180,
    // The frames the analyser buffers, the frame being analysed being the first.
    kLpcAf = 3,
    kLpcVoicingFrames = kLpcAf + 1,
    kLpcCurrentFrame = kLpcAf - 1,

    // Fortran index ranges of the analysis buffers.
    kLpcSbufLow = 181,
    kLpcBufHigh = 720,
    kLpcSbufSize = kLpcBufHigh - kLpcSbufLow + 1,
    kLpcLbufLow = 25,
    kLpcLbufSize = kLpcBufHigh - kLpcLbufLow + 1,
    kLpcPwinLow = 229,
    kLpcPwinHigh = 540,
    kLpcIvbufSize = kLpcPwinHigh - kLpcPwinLow + 1,
    kLpcNewSamplesFirst = kLpcBufHigh - kLpcFrame + 1,
    kLpcHighRange = kLpcAf * kLpcFrame,

    // The low-pass filter reads its input two samples before the window it writes.
    kLpfiltLength = 312,
    kLpfiltOutputFirst = kLpcBufHigh - kLpfiltLength + 1,
    kLpfiltInputFirst = kLpfiltOutputFirst - 2,
    kLpfiltPairs = 15,
    kLpfiltTaps = kLpfiltPairs + 1,
    kLpfiltSpan = 30,

    kIvfiltLength = kLpcIvbufSize,
    kIvfiltCorrelations = 3,
    kIvfiltLagStep = 4,
    kIvfiltSampleStep = 2,

    kLpcOnsetCapacity = 10,
    kLpcOnsetHistory = 16,
    // The onset is recorded this many samples before the sample that detects it.
    kLpcOnsetLag = 9,
    kLpcOnsetHold = 10,

    kLpcMaxWindow = 156,
    kLpcMinWindow = 90,
    kLpcDefaultVoicingStart = 307,
    kLpcDefaultVoicingEnd = 462,

    kLpcLags = 60,
    kLpcFineLags = 6,
    kLpcFineLagMin = 41,
    kLpcFineLagSpread = 3,
    kLpcOctaveLag = 80,
    kLpcOctavePointerShift = 20,
    kLpcMaxSearchSpread = 5,

    kLpcTrackDepth = 2,
    kLpcOctaveFirst = 20,
    kLpcOctaveLast = 40,
    kLpcOctaveStep = 10,

    kLpcVoicingParameters = 9,
    kLpcVoicingCoefficients = 10,
    kLpcSnrThresholds = 5,
    kLpcSnrClasses = 10,
    kLpcEnergyLimit = 32767,
    kLpcEnergyHistoryWeight = 63,
    kLpcEnergyScale = 8,
    kLpcEnergyGrowth = 3,
    kLpcInitialVoicedEnergy = 3000,
    kLpcInitialUnvoicedEnergy = 187,
    kLpcInitialLowUnvoicedEnergy = 93,
    kLpcSnrShift = 6,

    kLpcExcitationSize = 166,
    kLpcPulseLength = 25,
    kLpcNoiseShift = 6,
    kLpcNoiseLags = 5,
    kLpcNoiseLagLast = kLpcNoiseLags - 1,
    kLpcRandomOffset = 32768,
    kLpcRandomShift = 16,

    kLpcHammingSymbolMask = 255,
    kLpcHammingIndexMask = 127,
    kLpcHammingValueMask = 15,
    kLpcHammingValidBit = 16,

    kLpcChanwrFields = 13,
    kLpcChanwrBits = 53,
    kLpcChanwrSyncBit = kLpcChanwrBits,
    kLpcChanwrCodeMask = 0x7fff,
    kLpcChanwrWrite = 0,
    kLpcChanwrRead = 1,

    kLpcUnusedStateSize = 16,
};

// The index of the start and of the end of a window.
enum {
    kLpcWindowStart = 0,
    kLpcWindowEnd = 1,
};

// Onset bounds of the voicing window.
enum {
    kLpcBoundNone = 0,
    kLpcBoundLeft = 1,
    kLpcBoundRight = 2,
    kLpcBoundBoth = 3,
};

// NTSC-U/C: 0x003b6e78, PAL: 0x00435d38, the lags of the pitch search.
static const int g_anLpcTau[kLpcLags] = {
    20, 21, 22, 23, 24, 25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
    40, 42, 44, 46, 48, 50,  52,  54,  56,  58,  60,  62,  64,  66,  68,  70,  72,  74,  76,  78,
    80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156,
};

// NTSC-U/C: 0x003c1f60, PAL: 0x00440e20
static float g_afLpcPreemphasisState[2];

// NTSC-U/C: 0x003c1f68, PAL: 0x00440e28, the bias removed from the input samples.
static float g_fLpcBias;

// NTSC-U/C: 0x003c1f6c, PAL: 0x00440e2c, one more than the count of buffered onsets.
static int g_nLpcOnsetCount = 1;

// NTSC-U/C: 0x003c1f70, PAL: 0x00440e30
static int g_nLpcAnalysisFirst = 1;

// NTSC-U/C: 0x003c1f78, PAL: 0x00440e38, the glottal pulse of voiced excitation.
static const float g_afLpcPulse[kLpcPulseLength] = {
    8.0f,    -16.0f, 26.0f,  -48.0f,  86.0f,   -162.0f, 294.0f, -502.0f, 718.0f,
    -728.0f, 184.0f, 672.0f, -610.0f, -672.0f, 184.0f,  728.0f, 718.0f,  502.0f,
    294.0f,  162.0f, 86.0f,  48.0f,   26.0f,   16.0f,   8.0f,
};

// NTSC-U/C: 0x003c1fdc, PAL: 0x00440e9c, the length of the previous epoch.
static int g_nLpcPreviousEpochLength;

// NTSC-U/C: 0x003c1fe0, PAL: 0x00440ea0
static float g_fLpcPreviousEpochRms;

// NTSC-U/C: 0x003c1fe4, PAL: 0x00440ea4, the history of the low-pass filter of the voiced
// excitation.
static float g_fLpcPulseHistory1;

// NTSC-U/C: 0x003c1fe8, PAL: 0x00440ea8
static float g_fLpcPulseHistory2;

// NTSC-U/C: 0x003c1fec, PAL: 0x00440eac, the history of the high-pass filter of the voiced noise.
static float g_fLpcNoiseHistory1;

// NTSC-U/C: 0x003c1ff0, PAL: 0x00440eb0
static float g_fLpcNoiseHistory2;

// NTSC-U/C: 0x003c1ff8, PAL: 0x00440eb8, the sign bit of each reflection code.
static const int g_anLpcCodeSignBit[RT_LPC_ORDER] = {2, 4, 8, 8, 8, 8, 16, 16, 16, 16};

// NTSC-U/C: 0x003c2020, PAL: 0x00440ee0, the field each bit of a frame comes from, from one.
static const int g_anLpcBitOrder[kLpcChanwrBits] = {
    13, 12, 11, 1,  2,  13, 12, 11, 1, 2,  13, 10, 11, 2,  1, 10, 13, 12,
    11, 10, 2,  13, 12, 11, 10, 2,  1, 12, 7,  6,  1,  10, 9, 8,  7,  4,
    6,  9,  8,  7,  5,  1,  9,  8,  4, 6,  1,  5,  9,  8,  7, 5,  6,
};

// NTSC-U/C: 0x003c20f4, PAL: 0x00440fb4, the input history of the de-emphasis filter.
static float g_fLpcDeemphasisInput1;

// NTSC-U/C: 0x003c20f8, PAL: 0x00440fb8
static float g_fLpcDeemphasisInput2;

// NTSC-U/C: 0x003c20fc, PAL: 0x00440fbc, the output history of the de-emphasis filter.
static float g_fLpcDeemphasisOutput1;

// NTSC-U/C: 0x003c2100, PAL: 0x00440fc0
static float g_fLpcDeemphasisOutput2;

// NTSC-U/C: 0x003c2104, PAL: 0x00440fc4
static float g_fLpcDeemphasisOutput3;

// NTSC-U/C: 0x003c2108, PAL: 0x00440fc8
static float g_fLpcDeemphasisOutput4;

// NTSC-U/C: 0x003c210c, PAL: 0x00440fcc, the column of the pitch tracker for the current frame.
static float g_fLpcTrackColumn;

// NTSC-U/C: 0x003c2110, the decoded value of each Hamming symbol, with bit 4 set when valid.
static const int g_anLpcHammingTable[kLpcHammingIndexMask + 1] = {
    16, 0,  0,  3,  0,  5,  14, 7,  0,  9,  14, 11, 14, 13, 30, 14, 0,  9,  2,  7,  4,  7,
    7,  23, 9,  25, 10, 9,  12, 9,  14, 7,  0,  5,  2,  11, 5,  21, 6,  5,  8,  11, 11, 27,
    12, 5,  14, 11, 2,  1,  18, 2,  12, 5,  2,  7,  12, 9,  2,  11, 28, 12, 12, 15, 0,  3,
    3,  19, 4,  13, 6,  3,  8,  13, 10, 3,  13, 29, 14, 13, 4,  1,  10, 3,  20, 4,  4,  7,
    10, 9,  26, 10, 4,  13, 10, 15, 8,  1,  6,  3,  6,  5,  22, 6,  24, 8,  8,  11, 8,  13,
    6,  15, 1,  17, 2,  1,  4,  1,  6,  15, 8,  1,  10, 15, 12, 15, 15, 31,
};

// NTSC-U/C: 0x003c2310, PAL: 0x004411d0, the history of the first section of the high-pass filter.
static float g_fLpcHighPass1Z1;

// NTSC-U/C: 0x003c2314, PAL: 0x004411d4
static float g_fLpcHighPass1Z2;

// NTSC-U/C: 0x003c2318, PAL: 0x004411d8, the history of the second section of the high-pass filter.
static float g_fLpcHighPass2Z1;

// NTSC-U/C: 0x003c231c, PAL: 0x004411dc
static float g_fLpcHighPass2Z2;

// NTSC-U/C: 0x003c2320, PAL: 0x004411e0, the pivot below which the covariance solver gives up.
static float g_fLpcPivotThreshold = -9.0f;

// NTSC-U/C: 0x003c2324, PAL: 0x004411e4, the taps of the 31-tap low-pass filter, the centre tap
// last.
static const float g_afLpcLowPassTaps[kLpfiltTaps] = {
    -0.009720198f,
    -0.010517999f,
    -0.008347964f,
    0.0005860774f,
    0.013089209f,
    0.021705221f,
    0.018416123f,
    0.000339723f,
    -0.026079707f,
    -0.04555637f,
    -0.040306855f,
    0.00050298346f,
    0.07292628f,
    0.15720089f,
    0.22472885f,
    0.25053596f,
};

// NTSC-U/C: 0x003c2364, PAL: 0x00441224, the smoothed correlation of the onset detector.
static float g_fLpcOnsetNumerator;

// NTSC-U/C: 0x003c2368, PAL: 0x00441228, the smoothed energy of the onset detector.
static float g_fLpcOnsetDenominator = 1.0f;

// NTSC-U/C: 0x003c236c, PAL: 0x0044122c
static float g_fLpcOnsetSum;

// NTSC-U/C: 0x003c2370, PAL: 0x00441230
static float g_fLpcOnsetPreviousSum;

// NTSC-U/C: 0x003c2374, PAL: 0x00441234, the oldest entry of the onset history, from one.
static int g_nLpcOnsetOldest = 1;

// NTSC-U/C: 0x003c2378, PAL: 0x00441238, the middle entry of the onset history, from one.
static int g_nLpcOnsetMiddle = 9;

// NTSC-U/C: 0x003c237c, PAL: 0x0044123c
static short g_wLpcOnsetHysteresis;

// NTSC-U/C: 0x003c2380, PAL: 0x00441240
static int g_nLpcSynthPreviousVoicing;

// NTSC-U/C: 0x003c2384, PAL: 0x00441244
static int g_nLpcSynthPreviousPitch;

// NTSC-U/C: 0x003c2388, PAL: 0x00441248
static float g_fLpcSynthPreviousRms;

// NTSC-U/C: 0x003c238c, PAL: 0x0044124c
static short g_wLpcSynthFirst = 1;

// NTSC-U/C: 0x003c2390, PAL: 0x00441250, the lagged Fibonacci noise generator.
static short g_awLpcNoise[kLpcNoiseLags] = {-21161, -8478, 30892, -10216, 16950};

// NTSC-U/C: 0x003c239c, PAL: 0x0044125c
static int g_nLpcNoiseLagB = 1;

// NTSC-U/C: 0x003c23a0, PAL: 0x00441260
static int g_nLpcNoiseLagA = 4;

// NTSC-U/C: 0x003c23a8, PAL: 0x00441268
static int g_nLpcSnrClassCount = kLpcSnrThresholds;

// NTSC-U/C: 0x003c23b0, PAL: 0x00441270, the voicing discriminant by parameter and then by noise
// class, the bias last.
static const float g_aafLpcVoicingCoefficients[kLpcVoicingCoefficients][kLpcSnrClasses] = {
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1714.0f, 874.0f, 510.0f, 500.0f, 500.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {-110.0f, -97.0f, -70.0f, -10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {334.0f, 300.0f, 250.0f, 200.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {-4096.0f, -4096.0f, -4096.0f, -4096.0f, -4096.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {-654.0f, -1021.0f, -1270.0f, -1300.0f, -1300.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {3752.0f, 2451.0f, 2194.0f, 2000.0f, 2000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {3769.0f, 2527.0f, 2491.0f, 2000.0f, 2000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1181.0f, -500.0f, -1500.0f, -2000.0f, -2500.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
};

// NTSC-U/C: 0x003c2540, PAL: 0x00441400, the voicing of the last four half frames.
static int g_nLpcVoicingHistory;

// NTSC-U/C: 0x003c2544, PAL: 0x00441404, the threshold of the zero-crossing count.
static float g_fLpcDither = 20.0f;

// NTSC-U/C: 0x003c2548, PAL: 0x00441408, the smoothed low-band unvoiced energy, scaled by eight.
static int g_nLpcSmoothLowUnvoiced;

// NTSC-U/C: 0x003c254c, PAL: 0x0044140c
static short g_wLpcVoicinFirst = 1;

// NTSC-U/C: 0x004357e4, the input buffer, indexed after subtracting kLpcSbufLow.
static float *g_pfLpcInbuf;

// NTSC-U/C: 0x004357e8, the pre-emphasised buffer, indexed after subtracting kLpcSbufLow.
static float *g_pfLpcPebuf;

// NTSC-U/C: 0x004357ec, the low-pass buffer, indexed after subtracting kLpcLbufLow.
static float *g_pfLpcLpbuf;

// NTSC-U/C: 0x004357f0, the inverse-filtered buffer, indexed after subtracting kLpcPwinLow.
static float *g_pfLpcIvbuf;

// NTSC-U/C: 0x00528f20, the onset bound of the voicing window of each frame.
static int g_anLpcOnsetBound[kLpcAf];

// NTSC-U/C: 0x00528f30
static int g_aanLpcEnergyWindow[2][kLpcAf];

// NTSC-U/C: 0x00528f48
static int g_anLpcOnsets[kLpcOnsetCapacity];

// NTSC-U/C: 0x00528f70
static int g_nLpcSyncBit;

// NTSC-U/C: 0x00528f74, the smoothed minimum of the difference function.
static float g_fLpcAlphax;

// NTSC-U/C: 0x00528f78
static float g_fLpcInvertSave;

// NTSC-U/C: 0x00528f80
static float g_fLpcOnsetCorrelation;

// NTSC-U/C: 0x00528f88
static float g_afLpcOnsetHistory[kLpcOnsetHistory];

// NTSC-U/C: 0x00528fc8
static int g_nLpcLastOnset;

// NTSC-U/C: 0x00528fd0
static float g_afLpcSynthPreviousRc[RT_LPC_ORDER];

// NTSC-U/C: 0x00528ff8
static float g_afLpcSynthSavedRc[RT_LPC_ORDER];

// NTSC-U/C: 0x00529020, the samples of the frame not yet covered by an epoch.
static int g_nLpcSynthCarry;

// NTSC-U/C: 0x00529024
static short g_wLpcAllVoiced;

// NTSC-U/C: 0x00529028
static int g_anLpcFineTau[kLpcFineLags];

// NTSC-U/C: 0x00529040, the estimated signal-to-noise ratio.
static float g_fLpcSnr;

// NTSC-U/C: 0x00529044
static float g_fLpcAmdfRatio;

// NTSC-U/C: 0x00529048, the voicing discriminant by half frame and then by frame.
static float g_aafLpcVoicingValue[2][kLpcAf];

// NTSC-U/C: 0x00529060
static int g_nLpcLowVoicedEnergy;

// NTSC-U/C: 0x00529064
static int g_nLpcLowUnvoicedEnergy;

// NTSC-U/C: 0x00529068
static int g_nLpcFullVoicedEnergy;

// NTSC-U/C: 0x0052906c
static int g_nLpcFullUnvoicedEnergy;

// NTSC-U/C: 0x00529070
static int g_nLpcPreviousFullEnergy;

// NTSC-U/C: 0x00529074
static int g_nLpcPreviousLowEnergy;

// NTSC-U/C: 0x00529078, the smoothed full-band unvoiced energy, scaled by eight.
static int g_nLpcSmoothFullUnvoiced;

// NTSC-U/C: 0x005468c8
static float g_afLpcLpbuf[kLpcLbufSize];

// NTSC-U/C: 0x005473a8
static float g_afLpcIvbuf[kLpcIvbufSize];

// NTSC-U/C: 0x00547888
static float g_afLpcPebuf[kLpcSbufSize];

// NTSC-U/C: 0x005480f8
static float g_afLpcInbuf[kLpcSbufSize];

// NTSC-U/C: 0x00548968
static int g_aanLpcVoicingWindow[2][kLpcAf];

// NTSC-U/C: 0x00548980
static int g_aanLpcAnalysisWindow[2][kLpcAf];

// NTSC-U/C: 0x00548998, the voicing decision by half frame and then by frame.
static int g_aanLpcVoicing[2][kLpcVoicingFrames];

// NTSC-U/C: 0x005489b8
static float g_afLpcRmsBuffer[kLpcAf];

// NTSC-U/C: 0x005489c8
static float g_afLpcPsi[RT_LPC_ORDER];

// NTSC-U/C: 0x005489f0, the reflection coefficients by coefficient and then by frame.
static float g_aafLpcRcBuffer[RT_LPC_ORDER][kLpcAf];

// NTSC-U/C: 0x00548a68
static float g_afLpcAmdf[kLpcLags];

// NTSC-U/C: 0x00548b58
static float g_aafLpcPhi[RT_LPC_ORDER][RT_LPC_ORDER];

// NTSC-U/C: 0x00548ce8
static float g_afLpcExcitation[kLpcExcitationSize];

// NTSC-U/C: 0x00548f80
static float g_afLpcFilteredExcitation[kLpcExcitationSize];

// NTSC-U/C: 0x005492b0, the cost of each lag in the pitch tracker.
static float g_afLpcTrackCost[kLpcLags];

// NTSC-U/C: 0x005493a0, the best predecessor of each lag by frame.
static int g_aanLpcTrackPointer[kLpcLags][kLpcTrackDepth];

// NTSC-U/C: 0x00549580, cleared at reset and never read.
static int g_anLpcUnusedState[kLpcUnusedStateSize];

// NTSC-U/C: 0x00549870, the thresholds of the noise classes.
static float g_afLpcSnrThreshold[kLpcSnrThresholds];

// NTSC-U/C: 0x002f45c4, PAL: 0x002fe534
static void rtLpc10Dcbias(int length, const float *speech, float *output) {
    int i;
    float bias;

    bias = 0.0f;
    for (i = 0; i < length; ++i) {
        bias += speech[i];
    }
    bias /= (float)length;
    for (i = 0; i < length; ++i) {
        output[i] = speech[i] - bias;
    }
}

// NTSC-U/C: 0x002f4824, PAL: 0x002fe794
static void rtLpc10Dyptrk(const float *amdf, int minPointer, int voice, int *pitch, int *minIndex) {
    int i;
    int j;
    int column;
    int best;
    int path[kLpcTrackDepth];
    float alpha;
    float cost;
    float minCost;
    float maxCost;

    if (voice == 1) {
        g_fLpcAlphax = (float)(g_fLpcAlphax * 0.75 + amdf[minPointer - 1] * 0.5);
    } else {
        g_fLpcAlphax *= 0.984375f;
    }
    alpha = g_fLpcAlphax * 0.0625f;
    if (voice == 0 && g_fLpcAlphax < 128.0f) {
        alpha = 8.0f;
    }

    column = (int)(g_fLpcTrackColumn + 1.0f);
    g_aanLpcTrackPointer[0][column - 1] = 1;
    best = 1;
    cost = g_afLpcTrackCost[0];
    for (i = 1; i <= kLpcLags; ++i) {
        cost += alpha;
        if (cost < g_afLpcTrackCost[i - 1]) {
            g_afLpcTrackCost[i - 1] = cost;
            g_aanLpcTrackPointer[i - 1][column - 1] = best;
        } else {
            cost = g_afLpcTrackCost[i - 1];
            g_aanLpcTrackPointer[i - 1][column - 1] = i;
            best = i;
        }
    }
    i = best - 1;
    cost = g_afLpcTrackCost[i];
    while (i > 0) {
        cost += alpha;
        if (cost < g_afLpcTrackCost[i - 1]) {
            g_afLpcTrackCost[i - 1] = cost;
            g_aanLpcTrackPointer[i - 1][column - 1] = best;
        } else {
            best = g_aanLpcTrackPointer[i - 1][column - 1];
            i = best;
            cost = g_afLpcTrackCost[i - 1];
        }
        --i;
    }

    g_afLpcTrackCost[0] = (float)(amdf[0] * 0.5) + g_afLpcTrackCost[0];
    minCost = g_afLpcTrackCost[0];
    maxCost = minCost;
    *minIndex = 1;
    for (i = 2; i <= kLpcLags; ++i) {
        g_afLpcTrackCost[i - 1] = (float)(amdf[i - 1] * 0.5) + g_afLpcTrackCost[i - 1];
        if (g_afLpcTrackCost[i - 1] > maxCost) {
            maxCost = g_afLpcTrackCost[i - 1];
        }
        if (g_afLpcTrackCost[i - 1] < minCost) {
            *minIndex = i;
            minCost = g_afLpcTrackCost[i - 1];
        }
    }
    for (i = 0; i < kLpcLags; ++i) {
        g_afLpcTrackCost[i] -= minCost;
    }
    maxCost -= minCost;

    j = 0;
    for (i = kLpcOctaveFirst; i <= kLpcOctaveLast; i += kLpcOctaveStep) {
        if (*minIndex > i && g_afLpcTrackCost[*minIndex - i - 1] < maxCost * 0.25) {
            j = i;
        }
    }
    *minIndex -= j;

    j = (int)g_fLpcTrackColumn;
    *pitch = *minIndex;
    for (i = 1; i <= kLpcTrackDepth; ++i) {
        j = j % kLpcTrackDepth + 1;
        *pitch = g_aanLpcTrackPointer[*pitch - 1][j - 1];
        path[i - 1] = *pitch;
    }
    (void)path; // Yes, the binary stores the path and never reads it.
    g_fLpcTrackColumn = (float)(((int)g_fLpcTrackColumn + kLpcTrackDepth - 1) % kLpcTrackDepth);
}

// NTSC-U/C: 0x002f4cd8, PAL: 0x002fec48
static void rtLpc10Energy(int length, const float *speech, float *rms) {
    int i;

    *rms = 0.0f;
    for (i = 0; i < length; ++i) {
        *rms = speech[i] * speech[i] + *rms;
    }
    *rms = (float)sqrt(*rms / (float)length);
}

// NTSC-U/C: 0x002f5250, PAL: 0x002ff1c0
static void rtLpc10Invert(float phi[][RT_LPC_ORDER], const float *psi, float rc[][kLpcAf]) {
    int i;
    int j;
    int k;
    float clamped;

    for (j = 0; j < RT_LPC_ORDER; ++j) {
        for (k = 0; k < j; ++k) {
            g_fLpcInvertSave = phi[j][k] * phi[k][k];
            for (i = j; i < RT_LPC_ORDER; ++i) {
                phi[i][j] -= phi[i][k] * g_fLpcInvertSave;
            }
        }
        if ((float)fabs(phi[j][j]) < g_fLpcPivotThreshold) {
            break;
        }
        rc[j][kLpcCurrentFrame] = psi[j];
        for (k = 0; k < j; ++k) {
            rc[j][kLpcCurrentFrame] -= rc[k][kLpcCurrentFrame] * phi[j][k];
        }
        phi[j][j] = 1.0f / phi[j][j];
        rc[j][kLpcCurrentFrame] *= phi[j][j];
        clamped = (rc[j][kLpcCurrentFrame] < 0.99899995f) ? rc[j][kLpcCurrentFrame] : 0.99899995f;
        rc[j][kLpcCurrentFrame] = (clamped > -0.99899995f) ? clamped : -0.99899995f;
    }
    // Yes, the binary tests the pivot again here, and after a full pass it reads one entry past the
    // matrix; the clearing loop is empty then.
    if (j < RT_LPC_ORDER && (float)fabs(phi[j][j]) < g_fLpcPivotThreshold) {
        for (i = j; i < RT_LPC_ORDER; ++i) {
            rc[i][kLpcCurrentFrame] = 0.0f;
        }
    }
}

// NTSC-U/C: 0x002f5794, PAL: 0x002ff704
static void rtLpc10Ivfilt(const float *lpbuf, float *ivbuf, float *ivrc) {
    int i;
    int j;
    float r[kIvfiltCorrelations];
    float pc1;
    float pc2;

    for (i = 0; i < kIvfiltCorrelations; ++i) {
        r[i] = 0.0f;
        for (j = kIvfiltLagStep * (i + 1) + kIvfiltLength - kLpcFrame; j <= kIvfiltLength;
             j += kIvfiltSampleStep) {
            r[i] += lpbuf[j - 1] * lpbuf[j - 1 - kIvfiltLagStep * i];
        }
    }
    pc2 = 0.0f;
    pc1 = 0.0f;
    ivrc[0] = 0.0f;
    ivrc[1] = 0.0f;
    if (r[0] > 1e-06) {
        ivrc[0] = r[1] / r[0];
        ivrc[1] = (r[2] - ivrc[0] * r[1]) / (r[0] - ivrc[0] * r[1]);
        pc1 = ivrc[0] - ivrc[0] * ivrc[1];
        pc2 = ivrc[1];
    }
    for (i = kIvfiltLength + 1 - kLpcFrame; i <= kIvfiltLength; ++i) {
        ivbuf[i - 1] = lpbuf[i - 1] - pc1 * lpbuf[i - 1 - kIvfiltLagStep] -
                       pc2 * lpbuf[i - 1 - 2 * kIvfiltLagStep];
    }
}

// NTSC-U/C: 0x002f58f4, PAL: 0x002ff864
static void rtLpc10Lpfilt(const float *inbuf, float *lpbuf) {
    int j;
    int k;

    for (j = kLpfiltLength + 1 - kLpcFrame; j <= kLpfiltLength; ++j) {
        lpbuf[j - 1] = (inbuf[j - 1] + inbuf[j - 1 - kLpfiltSpan]) * g_afLpcLowPassTaps[0];
        for (k = 1; k < kLpfiltPairs; ++k) {
            lpbuf[j - 1] =
                (inbuf[j - 1 - k] + inbuf[j - 1 - kLpfiltSpan + k]) * g_afLpcLowPassTaps[k] +
                lpbuf[j - 1];
        }
        lpbuf[j - 1] =
            g_afLpcLowPassTaps[kLpfiltPairs] * inbuf[j - 1 - kLpfiltPairs] + lpbuf[j - 1];
    }
}

// NTSC-U/C: 0x002f5b78, PAL: 0x002ffae8
static void
rtLpc10Mload(int windowEnd, const float *speech, float phi[][RT_LPC_ORDER], float *psi) {
    int i;
    int r;
    int c;
    int start;

    start = RT_LPC_ORDER + 1;
    for (r = 1; r <= RT_LPC_ORDER; ++r) {
        phi[r - 1][0] = 0.0f;
        for (i = start; i <= windowEnd; ++i) {
            phi[r - 1][0] = speech[i - 2] * speech[i - r - 1] + phi[r - 1][0];
        }
    }
    psi[RT_LPC_ORDER - 1] = 0.0f;
    for (i = start; i <= windowEnd; ++i) {
        psi[RT_LPC_ORDER - 1] =
            speech[i - 1] * speech[i - RT_LPC_ORDER - 1] + psi[RT_LPC_ORDER - 1];
    }
    for (r = 2; r <= RT_LPC_ORDER; ++r) {
        for (c = 2; c <= r; ++c) {
            phi[r - 1][c - 1] = phi[r - 2][c - 2] - speech[windowEnd - r] * speech[windowEnd - c] +
                                speech[start - r - 1] * speech[start - c - 1];
        }
    }
    for (c = 1; c < RT_LPC_ORDER; ++c) {
        psi[c - 1] = phi[c][0] - speech[start - 2] * speech[start - c - 2] +
                     speech[windowEnd - 1] * speech[windowEnd - c - 1];
    }
}

// NTSC-U/C: 0x002f5d6c, PAL: 0x002ffcdc
static void rtLpc10Onset(const float *pebuf, int *osbuf, int *osptr) {
    int i;

    if (g_wLpcOnsetHysteresis != 0) {
        g_nLpcLastOnset -= kLpcFrame;
    }
    for (i = kLpcNewSamplesFirst; i <= kLpcBufHigh; ++i) {
        g_fLpcOnsetNumerator = (float)((pebuf[i - kLpcSbufLow] * pebuf[i - 1 - kLpcSbufLow] +
                                        g_fLpcOnsetNumerator * 63.0) *
                                       0.015625);
        g_fLpcOnsetDenominator = (float)((pebuf[i - 1 - kLpcSbufLow] * pebuf[i - 1 - kLpcSbufLow] +
                                          g_fLpcOnsetDenominator * 63.0) *
                                         0.015625);
        if (g_fLpcOnsetDenominator != 0.0) {
            if (g_fLpcOnsetDenominator < (float)fabs(g_fLpcOnsetNumerator)) {
                g_fLpcOnsetCorrelation = (g_fLpcOnsetNumerator < 0.0f) ? -1.0f : 1.0f;
            } else {
                g_fLpcOnsetCorrelation = g_fLpcOnsetNumerator / g_fLpcOnsetDenominator;
            }
        }
        g_fLpcOnsetPreviousSum = g_afLpcOnsetHistory[g_nLpcOnsetOldest - 1];
        g_fLpcOnsetSum =
            g_fLpcOnsetSum - g_afLpcOnsetHistory[g_nLpcOnsetMiddle - 1] + g_fLpcOnsetCorrelation;
        g_afLpcOnsetHistory[g_nLpcOnsetMiddle - 1] = g_fLpcOnsetSum;
        g_afLpcOnsetHistory[g_nLpcOnsetOldest - 1] = g_fLpcOnsetCorrelation;
        g_nLpcOnsetOldest = g_nLpcOnsetOldest % kLpcOnsetHistory + 1;
        g_nLpcOnsetMiddle = g_nLpcOnsetMiddle % kLpcOnsetHistory + 1;
        if ((float)fabs(g_fLpcOnsetSum - g_fLpcOnsetPreviousSum) > 1.6999999f) {
            if (g_wLpcOnsetHysteresis == 0) {
                if (*osptr <= kLpcOnsetCapacity) {
                    osbuf[*osptr - 1] = i - kLpcOnsetLag;
                    ++*osptr;
                }
                g_wLpcOnsetHysteresis = 1;
            }
            g_nLpcLastOnset = i;
        } else if (g_wLpcOnsetHysteresis != 0 && i - g_nLpcLastOnset >= kLpcOnsetHold) {
            g_wLpcOnsetHysteresis = 0;
        }
    }
}

// NTSC-U/C: 0x002f6ab0, PAL: 0x00300a20
static void rtLpc10Placea(int pitch,
                          int voibuf[][kLpcVoicingFrames],
                          int obound,
                          int vwin[][kLpcAf],
                          int awin[][kLpcAf],
                          int ewin[][kLpcAf]) {
    int i;
    int j;
    int k;
    int winv;
    int ephase;

    g_wLpcAllVoiced = (short)(voibuf[1][1] == 1);
    g_wLpcAllVoiced = (short)(g_wLpcAllVoiced != 0 && voibuf[0][kLpcCurrentFrame] == 1);
    g_wLpcAllVoiced = (short)(g_wLpcAllVoiced != 0 && voibuf[1][kLpcCurrentFrame] == 1);
    g_wLpcAllVoiced = (short)(g_wLpcAllVoiced != 0 && voibuf[0][kLpcAf] == 1);
    g_wLpcAllVoiced = (short)(g_wLpcAllVoiced != 0 && voibuf[1][kLpcAf] == 1);
    winv = voibuf[0][kLpcAf] == 1 || voibuf[1][kLpcAf] == 1;
    if (g_wLpcAllVoiced != 0 || (winv && obound == kLpcBoundNone)) {
        i = (kLpcSbufLow + pitch - 1 - awin[kLpcWindowStart][1]) / pitch;
        i = i * pitch + awin[kLpcWindowStart][1];
        k = (int)((vwin[kLpcWindowStart][kLpcCurrentFrame] + vwin[kLpcWindowEnd][kLpcCurrentFrame] +
                   1 - kLpcMaxWindow) *
                  0.5);
        awin[kLpcWindowStart][kLpcCurrentFrame] =
            i + (int)(float)((float)(k - i) / (float)pitch + 0.5) * pitch;
        awin[kLpcWindowEnd][kLpcCurrentFrame] =
            awin[kLpcWindowStart][kLpcCurrentFrame] + kLpcMaxWindow - 1;
        if (obound >= kLpcBoundRight &&
            awin[kLpcWindowEnd][kLpcCurrentFrame] > vwin[kLpcWindowEnd][kLpcCurrentFrame]) {
            awin[kLpcWindowStart][kLpcCurrentFrame] -= pitch;
            awin[kLpcWindowEnd][kLpcCurrentFrame] -= pitch;
        }
        if ((obound == kLpcBoundLeft || obound == kLpcBoundBoth) &&
            awin[kLpcWindowStart][kLpcCurrentFrame] < vwin[kLpcWindowStart][kLpcCurrentFrame]) {
            awin[kLpcWindowStart][kLpcCurrentFrame] += pitch;
            awin[kLpcWindowEnd][kLpcCurrentFrame] += pitch;
        }
        while (awin[kLpcWindowEnd][kLpcCurrentFrame] > kLpcHighRange) {
            awin[kLpcWindowStart][kLpcCurrentFrame] -= pitch;
            awin[kLpcWindowEnd][kLpcCurrentFrame] -= pitch;
        }
        while (awin[kLpcWindowStart][kLpcCurrentFrame] < kLpcSbufLow) {
            awin[kLpcWindowStart][kLpcCurrentFrame] += pitch;
            awin[kLpcWindowEnd][kLpcCurrentFrame] += pitch;
        }
        ephase = 1;
    } else {
        awin[kLpcWindowStart][kLpcCurrentFrame] = vwin[kLpcWindowStart][kLpcCurrentFrame];
        awin[kLpcWindowEnd][kLpcCurrentFrame] = vwin[kLpcWindowEnd][kLpcCurrentFrame];
        ephase = 0;
    }

    j = (awin[kLpcWindowEnd][kLpcCurrentFrame] - awin[kLpcWindowStart][kLpcCurrentFrame] + 1) /
        pitch * pitch;
    if (j == 0 || !winv) {
        ewin[kLpcWindowStart][kLpcCurrentFrame] = vwin[kLpcWindowStart][kLpcCurrentFrame];
        ewin[kLpcWindowEnd][kLpcCurrentFrame] = vwin[kLpcWindowEnd][kLpcCurrentFrame];
    } else if (!ephase && obound == kLpcBoundRight) {
        ewin[kLpcWindowStart][kLpcCurrentFrame] = awin[kLpcWindowEnd][kLpcCurrentFrame] - j + 1;
        ewin[kLpcWindowEnd][kLpcCurrentFrame] = awin[kLpcWindowEnd][kLpcCurrentFrame];
    } else {
        ewin[kLpcWindowStart][kLpcCurrentFrame] = awin[kLpcWindowStart][kLpcCurrentFrame];
        ewin[kLpcWindowEnd][kLpcCurrentFrame] = awin[kLpcWindowStart][kLpcCurrentFrame] + j - 1;
    }
}

// NTSC-U/C: 0x002f6e18, PAL: 0x00300d88
static void rtLpc10Placev(const int *osbuf, int osptr, int *obound, int vwin[][kLpcAf]) {
    int i;
    int q;
    int lowRange;
    int lastOnset;
    int critical;
    int limit;

    lowRange = vwin[kLpcWindowEnd][1] + 1;
    if (lowRange < kLpcSbufLow) {
        lowRange = kLpcSbufLow;
    }
    for (lastOnset = osptr - 1; lastOnset >= 1; --lastOnset) {
        if (osbuf[lastOnset - 1] <= kLpcHighRange) {
            break;
        }
    }
    ++lastOnset;

    if (lastOnset <= 1 || osbuf[lastOnset - 2] < lowRange) {
        vwin[kLpcWindowStart][kLpcCurrentFrame] = vwin[kLpcWindowEnd][1] + 1;
        if (vwin[kLpcWindowStart][kLpcCurrentFrame] < kLpcDefaultVoicingStart) {
            vwin[kLpcWindowStart][kLpcCurrentFrame] = kLpcDefaultVoicingStart;
        }
        vwin[kLpcWindowEnd][kLpcCurrentFrame] =
            vwin[kLpcWindowStart][kLpcCurrentFrame] + kLpcMaxWindow - 1;
        *obound = kLpcBoundNone;
        return;
    }

    for (q = lastOnset - 1; q >= 1; --q) {
        if (osbuf[q - 1] < lowRange) {
            break;
        }
    }
    ++q;

    critical = 0;
    for (i = q + 1; i <= lastOnset - 1; ++i) {
        if (osbuf[i - 1] - osbuf[q - 1] >= kLpcMinWindow) {
            critical = 1;
            break;
        }
    }
    limit = lowRange + kLpcMinWindow - 1;
    if (limit < (kLpcAf - 1) * kLpcFrame) {
        limit = (kLpcAf - 1) * kLpcFrame;
    }
    if (!critical && osbuf[q - 1] > limit) {
        vwin[kLpcWindowEnd][kLpcCurrentFrame] = osbuf[q - 1] - 1;
        vwin[kLpcWindowStart][kLpcCurrentFrame] = osbuf[q - 1] - kLpcMaxWindow;
        if (vwin[kLpcWindowStart][kLpcCurrentFrame] < lowRange) {
            vwin[kLpcWindowStart][kLpcCurrentFrame] = lowRange;
        }
        *obound = kLpcBoundRight;
        return;
    }

    vwin[kLpcWindowStart][kLpcCurrentFrame] = osbuf[q - 1];
    for (;;) {
        ++q;
        if (q >= lastOnset ||
            osbuf[q - 1] > vwin[kLpcWindowStart][kLpcCurrentFrame] + kLpcMaxWindow) {
            break;
        }
        if (osbuf[q - 1] >= vwin[kLpcWindowStart][kLpcCurrentFrame] + kLpcMinWindow) {
            vwin[kLpcWindowEnd][kLpcCurrentFrame] = osbuf[q - 1] - 1;
            *obound = kLpcBoundBoth;
            return;
        }
    }
    vwin[kLpcWindowEnd][kLpcCurrentFrame] =
        vwin[kLpcWindowStart][kLpcCurrentFrame] + kLpcMaxWindow - 1;
    if (vwin[kLpcWindowEnd][kLpcCurrentFrame] >= kLpcHighRange) {
        vwin[kLpcWindowEnd][kLpcCurrentFrame] = kLpcHighRange;
    }
    *obound = kLpcBoundLeft;
}

// NTSC-U/C: 0x002f6fe4, PAL: 0x00300f54
static void
rtLpc10Preemp(const float *input, float *output, int count, float coefficient, float *state) {
    int i;
    float sample;

    for (i = 0; i < count; ++i) {
        sample = input[i] - state[0] + coefficient * state[1];
        state[1] = state[0];
        state[0] = input[i];
        output[i] = sample;
    }
}

// NTSC-U/C: 0x002f7038, PAL: 0x00300fa8
static int rtLpc10Random(void) {
    int value;

    g_awLpcNoise[g_nLpcNoiseLagA] += g_awLpcNoise[g_nLpcNoiseLagB];
    value = g_awLpcNoise[g_nLpcNoiseLagA];
    --g_nLpcNoiseLagA;
    if (g_nLpcNoiseLagA < 0) {
        g_nLpcNoiseLagA = kLpcNoiseLagLast;
    }
    --g_nLpcNoiseLagB;
    if (g_nLpcNoiseLagB < 0) {
        g_nLpcNoiseLagB = kLpcNoiseLagLast;
    }
    return value;
}

// NTSC-U/C: 0x002f70b8, PAL: 0x00301028
static void rtLpc10Rcchk(float rc[][kLpcAf]) {
    int i;

    for (i = 0; i < RT_LPC_ORDER; ++i) {
        if ((float)fabs(rc[i][kLpcCurrentFrame]) > 0.99) {
            break;
        }
    }
    // Yes, the binary tests the coefficient that ended the search again.
    if (i < RT_LPC_ORDER && (float)fabs(rc[i][kLpcCurrentFrame]) > 0.99) {
        for (i = 0; i < RT_LPC_ORDER; ++i) {
            rc[i][kLpcCurrentFrame] = rc[i][kLpcCurrentFrame - 1];
        }
    }
}

// NTSC-U/C: 0x002f7210, PAL: 0x00301180
static void rtLpc10Tbdm(const float *speech,
                        const int *tau,
                        float *amdf,
                        int *minPointer,
                        int *maxPointer,
                        int *minTau) {
    int i;
    int pointer;
    int count;
    int minAmdf;
    int fineMin;
    int fineMax;
    int half;
    float fineAmdf[kLpcFineLags];

    // The minimum and maximum positions from rtLpc10Difmag() count from 1.
    rtLpc10Difmag(speech, tau, kLpcLags, tau[kLpcLags - 1], amdf, minPointer, maxPointer);
    *minTau = tau[*minPointer - 1];
    minAmdf = (int)amdf[*minPointer - 1];

    count = 0;
    pointer = *minPointer - 2;
    for (i = (*minTau - kLpcFineLagSpread < kLpcFineLagMin) ? kLpcFineLagMin :
                                                              *minTau - kLpcFineLagSpread;
         i <= ((*minTau + kLpcFineLagSpread < tau[kLpcLags - 1]) ? *minTau + kLpcFineLagSpread :
                                                                   tau[kLpcLags - 1]);
         ++i) {
        while (tau[pointer - 1] < i) {
            ++pointer;
        }
        if (tau[pointer - 1] != i) {
            ++count;
            g_anLpcFineTau[count - 1] = i;
        }
    }
    if (count > 0) {
        rtLpc10Difmag(
            speech, g_anLpcFineTau, count, tau[kLpcLags - 1], fineAmdf, &fineMin, &fineMax);
        if (fineAmdf[fineMin - 1] < (float)minAmdf) {
            *minTau = g_anLpcFineTau[fineMin - 1];
            minAmdf = (int)fineAmdf[fineMin - 1];
        }
    }

    if (*minTau >= kLpcOctaveLag) {
        half = (int)(*minTau * 0.5);
        if ((half & 1) != 0) {
            count = 1;
            g_anLpcFineTau[0] = half;
        } else {
            count = 2;
            g_anLpcFineTau[0] = half - 1;
            g_anLpcFineTau[1] = half + 1;
        }
        rtLpc10Difmag(
            speech, g_anLpcFineTau, count, tau[kLpcLags - 1], fineAmdf, &fineMin, &fineMax);
        if (fineAmdf[fineMin - 1] < (float)minAmdf) {
            *minTau = g_anLpcFineTau[fineMin - 1];
            minAmdf = (int)fineAmdf[fineMin - 1];
            *minPointer -= kLpcOctavePointerShift;
        }
    }

    amdf[*minPointer - 1] = (float)minAmdf;
    *maxPointer = *minPointer - kLpcMaxSearchSpread;
    if (*maxPointer < 1) {
        *maxPointer = 1;
    }
    for (i = *maxPointer + 1;
         i <= ((*minPointer + kLpcMaxSearchSpread <= kLpcLags) ? *minPointer + kLpcMaxSearchSpread :
                                                                 kLpcLags);
         ++i) {
        if (amdf[*maxPointer - 1] < amdf[i - 1]) {
            *maxPointer = i;
        }
    }
}

// NTSC-U/C: 0x002f7f3c, PAL: 0x00301eac
static void rtLpc10Vparms(int vwin[][kLpcAf],
                          const float *inbuf,
                          const float *lpbuf,
                          int half,
                          float *dither,
                          int minTau,
                          int *zc,
                          int *lbe,
                          int *fbe,
                          float *qs,
                          float *rc1,
                          float *arB,
                          float *arF) {
    int i;
    int length;
    int count;
    int start;
    int stop;
    float apRms;
    float lpRms;
    float ePre;
    float e0ap;
    float e0;
    float eB;
    float eF;
    float rF;
    float rB;
    float oldSign;
    float sign;
    double scale;
    double value;
    double backward;
    double forward;

    apRms = 0.0f;
    lpRms = 0.0f;
    ePre = 0.0f;
    e0ap = 0.0f;
    *rc1 = 0.0f;
    e0 = 0.0f;
    eB = 0.0f;
    eF = 0.0f;
    rF = 0.0f;
    rB = 0.0f;
    *zc = 0;
    length = vwin[kLpcWindowEnd][kLpcCurrentFrame] - vwin[kLpcWindowStart][kLpcCurrentFrame] + 1;
    count = length >> 1;
    start = vwin[kLpcWindowStart][kLpcCurrentFrame] + (half - 1) * count + 1;
    stop = start + count - 1;
    oldSign = (inbuf[start - 1 - kLpcSbufLow] - *dither < 0.0f) ? -1.0f : 1.0f;
    for (i = start; i <= stop; ++i) {
        lpRms += (float)fabs(lpbuf[i - kLpcLbufLow]);
        apRms += (float)fabs(inbuf[i - kLpcSbufLow]);
        ePre += (float)fabs(inbuf[i - kLpcSbufLow] - inbuf[i - 1 - kLpcSbufLow]);
        e0ap += inbuf[i - kLpcSbufLow] * inbuf[i - kLpcSbufLow];
        *rc1 += inbuf[i - kLpcSbufLow] * inbuf[i - 1 - kLpcSbufLow];
        e0 += lpbuf[i - kLpcLbufLow] * lpbuf[i - kLpcLbufLow];
        eB += lpbuf[i - minTau - kLpcLbufLow] * lpbuf[i - minTau - kLpcLbufLow];
        eF += lpbuf[i + minTau - kLpcLbufLow] * lpbuf[i + minTau - kLpcLbufLow];
        rF += lpbuf[i - kLpcLbufLow] * lpbuf[i + minTau - kLpcLbufLow];
        rB += lpbuf[i - kLpcLbufLow] * lpbuf[i - minTau - kLpcLbufLow];
        sign = (inbuf[i - kLpcSbufLow] + *dither < 0.0f) ? -1.0f : 1.0f;
        if (sign != oldSign) {
            ++*zc;
            oldSign = -oldSign;
        }
        *dither = -*dither;
    }

    if (e0ap > 1.0) {
        *rc1 /= e0ap;
    }
    value = (double)apRms + (double)apRms;
    *qs = (float)((value > 1.0) ? ePre / value : (double)ePre);
    backward = (eB > 1.0) ? (double)eB : 1.0;
    *arB = (float)((e0 > 1.0) ? (double)(rB * rB) / (backward * e0) : (double)(rB * rB) / backward);
    forward = (eF > 1.0) ? rF / (double)eF : (double)rF;
    *arF = (float)((e0 > 1.0) ? forward * (rF / (double)e0) : forward * rF);

    scale = 90.0 / length;
    *zc = (int)((*zc << 1) * scale + 0.5);
    value = lpRms * 0.25 * scale + 0.5;
    *lbe = (value < kLpcEnergyLimit) ? (int)value : kLpcEnergyLimit;
    value = apRms * 0.25 * (90.0 / length) + 0.5;
    *fbe = (value < kLpcEnergyLimit) ? (int)value : kLpcEnergyLimit;
}

// The divisor 3000 is the expected voiced energy.
static inline double rtLpc10DitherLevel(void) {
    return sqrt((float)(g_nLpcLowUnvoicedEnergy * g_nLpcLowVoicedEnergy)) * 64.0 / 3000;
}

// Applies the smoothing rules of the voicing decision of the second half frame.
static inline void rtLpc10SmoothVoicing(const int *obound, int voibuf[][kLpcVoicingFrames]) {
    int onsetTransition;

    onsetTransition = ((obound[0] & kLpcBoundRight) != 0 || obound[1] == kLpcBoundLeft) &&
                      (obound[2] & kLpcBoundLeft) == 0;
    g_nLpcVoicingHistory =
        ((voibuf[0][1] * 2 + voibuf[1][1]) * 2 + voibuf[0][kLpcCurrentFrame]) * 2 +
        voibuf[1][kLpcCurrentFrame];
    switch (g_nLpcVoicingHistory) {
    case 1:
        if (onsetTransition && voibuf[0][kLpcAf] == 1) {
            voibuf[0][kLpcCurrentFrame] = 1;
        }
        break;
    case 2:
        if (voibuf[0][kLpcAf] == 0 || g_aafLpcVoicingValue[0][1] < -g_aafLpcVoicingValue[1][1]) {
            voibuf[0][kLpcCurrentFrame] = 0;
        } else {
            voibuf[1][kLpcCurrentFrame] = 1;
        }
        break;
    case 4:
        voibuf[1][1] = 0;
        break;
    case 5:
        if (g_aafLpcVoicingValue[1][0] < -g_aafLpcVoicingValue[0][1]) {
            voibuf[1][1] = 0;
        } else {
            voibuf[0][kLpcCurrentFrame] = 1;
        }
        break;
    case 6:
        if (voibuf[0][0] == 1 || voibuf[0][kLpcAf] == 1 ||
            g_aafLpcVoicingValue[0][0] < g_aafLpcVoicingValue[1][1]) {
            voibuf[1][kLpcCurrentFrame] = 1;
        } else {
            voibuf[0][1] = 1;
        }
        break;
    case 7:
        if (onsetTransition) {
            voibuf[1][1] = 0;
        }
        break;
    case 8:
        if (onsetTransition) {
            voibuf[1][1] = 1;
        }
        break;
    case 10:
        if (g_aafLpcVoicingValue[0][1] < -g_aafLpcVoicingValue[1][0]) {
            voibuf[0][kLpcCurrentFrame] = 0;
        } else {
            voibuf[1][1] = 1;
        }
        break;
    case 11:
        voibuf[1][1] = 1;
        break;
    case 12:
        if (voibuf[0][kLpcAf] == 0 && g_aafLpcVoicingValue[1][1] < -g_aafLpcVoicingValue[0][1]) {
            voibuf[1][kLpcCurrentFrame] = 0;
        } else {
            voibuf[0][kLpcCurrentFrame] = 1;
        }
        break;
    case 13:
        if (onsetTransition && voibuf[0][kLpcAf] == 0) {
            voibuf[0][kLpcCurrentFrame] = 0;
        }
        break;
    default:
        break;
    }
}

// NTSC-U/C: 0x002f7524, PAL: 0x00301494
static void rtLpc10Voicin(int vwin[][kLpcAf],
                          const float *inbuf,
                          const float *lpbuf,
                          int half,
                          float minAmdf,
                          float maxAmdf,
                          int minTau,
                          const float *ivrc,
                          const int *obound,
                          int voibuf[][kLpcVoicingFrames]) {
    int i;
    int snrClass;
    int zc;
    int lbe;
    int fbe;
    int limit;
    float qs;
    float rc1;
    float arB;
    float arF;
    float snr2;
    float discriminant;
    float value[kLpcVoicingParameters];

    if (g_wLpcVoicinFirst != 0) {
        g_nLpcLowVoicedEnergy = kLpcInitialVoicedEnergy;
        g_nLpcFullVoicedEnergy = kLpcInitialVoicedEnergy;
        g_nLpcFullUnvoicedEnergy = kLpcInitialUnvoicedEnergy;
        g_nLpcPreviousFullEnergy = kLpcInitialUnvoicedEnergy;
        g_nLpcLowUnvoicedEnergy = kLpcInitialLowUnvoicedEnergy;
        g_nLpcPreviousLowEnergy = kLpcInitialLowUnvoicedEnergy;
        g_fLpcSnr = (float)((g_nLpcFullVoicedEnergy / g_nLpcFullUnvoicedEnergy) << kLpcSnrShift);
        g_wLpcVoicinFirst = 0;
        g_afLpcSnrThreshold[0] = 600.0f;
        g_afLpcSnrThreshold[1] = 450.0f;
        g_afLpcSnrThreshold[2] = 300.0f;
        g_afLpcSnrThreshold[3] = 200.0f;
        g_afLpcSnrThreshold[4] = 0.0f;
        for (i = 0; i < kLpcAf; ++i) {
            g_aafLpcVoicingValue[1][i] = 0.0f;
            g_aafLpcVoicingValue[0][i] = 0.0f;
        }
    }

    if (half == 1) {
        g_aafLpcVoicingValue[0][0] = g_aafLpcVoicingValue[0][1];
        g_aafLpcVoicingValue[1][0] = g_aafLpcVoicingValue[1][1];
        g_aafLpcVoicingValue[0][1] = g_aafLpcVoicingValue[0][kLpcCurrentFrame];
        g_aafLpcVoicingValue[1][1] = g_aafLpcVoicingValue[1][kLpcCurrentFrame];
        g_fLpcAmdfRatio = (float)((minAmdf > 1.0) ? maxAmdf / (double)minAmdf : (double)maxAmdf);
    }

    rtLpc10Vparms(
        vwin, inbuf, lpbuf, half, &g_fLpcDither, minTau, &zc, &lbe, &fbe, &qs, &rc1, &arB, &arF);

    limit = (g_nLpcFullUnvoicedEnergy < 1) ? 1 : g_nLpcFullUnvoicedEnergy;
    g_fLpcSnr =
        (float)(int)(((float)g_nLpcFullVoicedEnergy / (float)limit + g_fLpcSnr) * 63.0f * 0.015625 +
                     0.5);
    limit = (g_nLpcLowUnvoicedEnergy < 1) ? 1 : g_nLpcLowUnvoicedEnergy;
    snr2 = (float)g_nLpcFullUnvoicedEnergy * g_fLpcSnr / (float)limit;
    for (snrClass = 1; snrClass < g_nLpcSnrClassCount; ++snrClass) {
        if (snr2 > g_afLpcSnrThreshold[snrClass - 1]) {
            break;
        }
    }

    value[0] = g_fLpcAmdfRatio;
    limit = (g_nLpcLowVoicedEnergy < 1) ? 1 : g_nLpcLowVoicedEnergy;
    value[1] = (float)lbe / (float)limit;
    value[2] = (float)zc;
    value[3] = rc1;
    value[4] = qs;
    value[5] = ivrc[1];
    value[6] = arB;
    value[7] = arF;
    value[8] = 0.0f;

    discriminant = g_aafLpcVoicingCoefficients[kLpcVoicingCoefficients - 1][snrClass - 1];
    for (i = 1; i < kLpcVoicingCoefficients; ++i) {
        discriminant += g_aafLpcVoicingCoefficients[i - 1][snrClass - 1] * value[i - 1];
    }
    g_aafLpcVoicingValue[half - 1][kLpcCurrentFrame] = discriminant;
    voibuf[half - 1][kLpcAf] = (g_aafLpcVoicingValue[half - 1][kLpcCurrentFrame] > 0.0) ? 1 : 0;

    if (half != 1) {
        rtLpc10SmoothVoicing(obound, voibuf);
    }

    if (voibuf[half - 1][kLpcAf] == 0) {
        limit = g_nLpcPreviousFullEnergy * kLpcEnergyGrowth;
        g_nLpcSmoothFullUnvoiced = (int)((g_nLpcSmoothFullUnvoiced * kLpcEnergyHistoryWeight +
                                          ((fbe < limit) ? fbe : limit) * kLpcEnergyScale) *
                                             0.015625 +
                                         0.5);
        g_nLpcFullUnvoicedEnergy = g_nLpcSmoothFullUnvoiced / kLpcEnergyScale;
        g_nLpcPreviousFullEnergy = fbe;
        limit = g_nLpcPreviousLowEnergy * kLpcEnergyGrowth;
        g_nLpcSmoothLowUnvoiced = (int)((g_nLpcSmoothLowUnvoiced * kLpcEnergyHistoryWeight +
                                         ((lbe < limit) ? lbe : limit) * kLpcEnergyScale) *
                                            0.015625 +
                                        0.5);
        g_nLpcLowUnvoicedEnergy = g_nLpcSmoothLowUnvoiced / kLpcEnergyScale;
        g_nLpcPreviousLowEnergy = lbe;
    } else {
        g_nLpcLowVoicedEnergy =
            (int)((g_nLpcLowVoicedEnergy * kLpcEnergyHistoryWeight + lbe) * 0.015625 + 0.5);
        g_nLpcFullVoicedEnergy =
            (int)((g_nLpcFullVoicedEnergy * kLpcEnergyHistoryWeight + fbe) * 0.015625 + 0.5);
    }

    // Yes, the binary evaluates the level again for each test.
    if (!(rtLpc10DitherLevel() > 1.0) || rtLpc10DitherLevel() < 20.0) {
        g_fLpcDither = (rtLpc10DitherLevel() > 1.0) ? (float)rtLpc10DitherLevel() : 1.0f;
    } else {
        g_fLpcDither = 20.0f;
    }
}

// Moves the analysis buffers and the per-frame parameters one frame back.
static inline void rtLpc10ShiftAnalysis(void) {
    int i;
    int j;
    int count;

    for (i = kLpcSbufLow; i <= kLpcBufHigh - kLpcFrame; ++i) {
        g_pfLpcInbuf[i - kLpcSbufLow] = g_pfLpcInbuf[i + kLpcFrame - kLpcSbufLow];
        g_pfLpcPebuf[i - kLpcSbufLow] = g_pfLpcPebuf[i + kLpcFrame - kLpcSbufLow];
    }
    for (i = kLpcPwinLow; i <= kLpcPwinHigh - kLpcFrame; ++i) {
        g_pfLpcIvbuf[i - kLpcPwinLow] = g_pfLpcIvbuf[i + kLpcFrame - kLpcPwinLow];
    }
    for (i = kLpcLbufLow; i <= kLpcBufHigh - kLpcFrame; ++i) {
        g_pfLpcLpbuf[i - kLpcLbufLow] = g_pfLpcLpbuf[i + kLpcFrame - kLpcLbufLow];
    }

    count = 1;
    for (i = 1; i <= g_nLpcOnsetCount - 1; ++i) {
        if (g_anLpcOnsets[i - 1] > kLpcFrame) {
            g_anLpcOnsets[count - 1] = g_anLpcOnsets[i - 1] - kLpcFrame;
            ++count;
        }
    }
    g_nLpcOnsetCount = count;

    g_aanLpcVoicing[0][0] = g_aanLpcVoicing[0][1];
    g_aanLpcVoicing[1][0] = g_aanLpcVoicing[1][1];
    for (i = 0; i < kLpcAf - 1; ++i) {
        g_aanLpcVoicingWindow[kLpcWindowStart][i] =
            g_aanLpcVoicingWindow[kLpcWindowStart][i + 1] - kLpcFrame;
        g_aanLpcVoicingWindow[kLpcWindowEnd][i] =
            g_aanLpcVoicingWindow[kLpcWindowEnd][i + 1] - kLpcFrame;
        g_aanLpcAnalysisWindow[kLpcWindowStart][i] =
            g_aanLpcAnalysisWindow[kLpcWindowStart][i + 1] - kLpcFrame;
        g_aanLpcAnalysisWindow[kLpcWindowEnd][i] =
            g_aanLpcAnalysisWindow[kLpcWindowEnd][i + 1] - kLpcFrame;
        g_anLpcOnsetBound[i] = g_anLpcOnsetBound[i + 1];
        g_aanLpcVoicing[0][i + 1] = g_aanLpcVoicing[0][i + 2];
        g_aanLpcVoicing[1][i + 1] = g_aanLpcVoicing[1][i + 2];
        g_afLpcRmsBuffer[i] = g_afLpcRmsBuffer[i + 1];
        for (j = 0; j < RT_LPC_ORDER; ++j) {
            g_aafLpcRcBuffer[j][i] = g_aafLpcRcBuffer[j][i + 1];
        }
    }
}

void rtLpc10Analys(const float *speech, int *voice, int *pitch, float *rms, float *rc) {
    int i;
    int length;
    int minPointer;
    int maxPointer;
    int minTau;
    int minIndex;
    float sum;
    float ivrc[2];
    float abuf[kLpcMaxWindow];

    if (g_nLpcAnalysisFirst != 0) {
        g_nLpcAnalysisFirst = 0;
        for (i = 0; i < kLpcOnsetCapacity; ++i) {
            g_anLpcOnsets[i] = 0;
        }
        for (i = 0; i < kLpcAf; ++i) {
            g_anLpcOnsetBound[i] = 0;
        }
    }
    rtLpc10ShiftAnalysis();

    sum = 0.0f;
    for (i = 0; i < kLpcFrame; ++i) {
        g_pfLpcInbuf[kLpcNewSamplesFirst + i - kLpcSbufLow] =
            (float)(speech[i] * 4096.0 - g_fLpcBias);
        sum += g_pfLpcInbuf[kLpcNewSamplesFirst + i - kLpcSbufLow];
    }
    if (sum > (float)kLpcFrame) {
        g_fLpcBias += 1.0f;
    }
    if (sum < (float)-kLpcFrame) {
        g_fLpcBias -= 1.0f;
    }

    rtLpc10Preemp(&g_pfLpcInbuf[kLpcNewSamplesFirst - kLpcSbufLow],
                  &g_pfLpcPebuf[kLpcNewSamplesFirst - kLpcSbufLow],
                  kLpcFrame,
                  0.39999998f,
                  g_afLpcPreemphasisState);
    rtLpc10Onset(g_pfLpcPebuf, g_anLpcOnsets, &g_nLpcOnsetCount);
    rtLpc10Placev(g_anLpcOnsets,
                  g_nLpcOnsetCount,
                  &g_anLpcOnsetBound[kLpcCurrentFrame],
                  g_aanLpcVoicingWindow);
    rtLpc10Lpfilt(&g_pfLpcInbuf[kLpfiltInputFirst - kLpcSbufLow],
                  &g_pfLpcLpbuf[kLpfiltOutputFirst - kLpcLbufLow]);
    rtLpc10Ivfilt(&g_pfLpcLpbuf[kLpcPwinLow - kLpcLbufLow], g_pfLpcIvbuf, ivrc);
    rtLpc10Tbdm(g_pfLpcIvbuf, g_anLpcTau, g_afLpcAmdf, &minPointer, &maxPointer, &minTau);
    for (i = 1; i <= 2; ++i) {
        rtLpc10Voicin(g_aanLpcVoicingWindow,
                      g_pfLpcInbuf,
                      g_pfLpcLpbuf,
                      i,
                      g_afLpcAmdf[minPointer - 1],
                      g_afLpcAmdf[maxPointer - 1],
                      minTau,
                      ivrc,
                      g_anLpcOnsetBound,
                      g_aanLpcVoicing);
    }
    rtLpc10Dyptrk(g_afLpcAmdf, minPointer, g_aanLpcVoicing[1][kLpcAf], pitch, &minIndex);
    rtLpc10Placea(g_anLpcTau[minIndex - 1],
                  g_aanLpcVoicing,
                  g_anLpcOnsetBound[kLpcCurrentFrame],
                  g_aanLpcVoicingWindow,
                  g_aanLpcAnalysisWindow,
                  g_aanLpcEnergyWindow);

    length = g_aanLpcAnalysisWindow[kLpcWindowEnd][kLpcCurrentFrame] -
             g_aanLpcAnalysisWindow[kLpcWindowStart][kLpcCurrentFrame] + 1;
    rtLpc10Dcbias(
        length,
        &g_pfLpcPebuf[g_aanLpcAnalysisWindow[kLpcWindowStart][kLpcCurrentFrame] - kLpcSbufLow],
        abuf);
    rtLpc10Energy(g_aanLpcEnergyWindow[kLpcWindowEnd][kLpcCurrentFrame] -
                      g_aanLpcEnergyWindow[kLpcWindowStart][kLpcCurrentFrame] + 1,
                  &abuf[g_aanLpcEnergyWindow[kLpcWindowStart][kLpcCurrentFrame] -
                        g_aanLpcAnalysisWindow[kLpcWindowStart][kLpcCurrentFrame]],
                  &g_afLpcRmsBuffer[kLpcCurrentFrame]);
    rtLpc10Mload(length, abuf, g_aafLpcPhi, g_afLpcPsi);
    rtLpc10Invert(g_aafLpcPhi, g_afLpcPsi, g_aafLpcRcBuffer);
    rtLpc10Rcchk(g_aafLpcRcBuffer);

    voice[0] = g_aanLpcVoicing[0][1];
    voice[1] = g_aanLpcVoicing[1][1];
    *rms = g_afLpcRmsBuffer[0];
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        rc[i] = g_aafLpcRcBuffer[i][0];
    }
}

void rtLpc10Bsynz(const float *predictor,
                  int pitch,
                  int voicing,
                  float *output,
                  float rms,
                  float ratio,
                  float gain) {
    int i;
    int j;
    int position;
    float scale;
    float sample;
    float sum;
    float energy;
    float pulse;
    float noise[kLpcExcitationSize];
    double quotient;

    quotient = g_fLpcPreviousEpochRms / (rms + 1e-06);
    scale = (quotient < 8.0) ? (float)quotient : 8.0f;
    g_fLpcPreviousEpochRms = rms;
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        g_afLpcFilteredExcitation[i] =
            scale * g_afLpcFilteredExcitation[g_nLpcPreviousEpochLength + i];
    }
    g_nLpcPreviousEpochLength = pitch;

    if (voicing == 0) {
        for (i = 0; i < pitch; ++i) {
            g_afLpcExcitation[RT_LPC_ORDER + i] = (float)(rtLpc10Random() >> kLpcNoiseShift);
        }
        position = ((rtLpc10Random() + kLpcRandomOffset) * (pitch - 1) >> kLpcRandomShift) +
                   RT_LPC_ORDER + 1;
        pulse = (float)(ratio * 85.5);
        if (pulse > 2000.0f) {
            pulse = 2000.0f;
        }
        g_afLpcExcitation[position - 1] = pulse + g_afLpcExcitation[position - 1];
        g_afLpcExcitation[position] -= pulse;
    } else {
        scale = (float)(sqrt((float)pitch) * 0x1.279cac95ed092p-3);
        for (i = 0; i < pitch; ++i) {
            g_afLpcExcitation[RT_LPC_ORDER + i] = 0.0f;
            // Yes, the binary also scales the word after the pulse for the 26th sample; the word
            // is the previous epoch length, a denormal that the FPU reads as zero.
            if (i < kLpcPulseLength) {
                g_afLpcExcitation[RT_LPC_ORDER + i] = scale * g_afLpcPulse[i];
            }
            sample = g_afLpcExcitation[RT_LPC_ORDER + i];
            g_afLpcExcitation[RT_LPC_ORDER + i] =
                0.125f * sample + 0.75f * g_fLpcPulseHistory1 + 0.125f * g_fLpcPulseHistory2;
            g_fLpcPulseHistory2 = g_fLpcPulseHistory1;
            g_fLpcPulseHistory1 = sample;
        }
        for (i = 0; i < pitch; ++i) {
            sample = (float)(rtLpc10Random() >> kLpcNoiseShift);
            noise[RT_LPC_ORDER + i] =
                -0.125f * sample + 0.25f * g_fLpcNoiseHistory1 + -0.125f * g_fLpcNoiseHistory2;
            g_fLpcNoiseHistory2 = g_fLpcNoiseHistory1;
            g_fLpcNoiseHistory1 = sample;
        }
        for (i = 0; i < pitch; ++i) {
            g_afLpcExcitation[RT_LPC_ORDER + i] += noise[RT_LPC_ORDER + i];
        }
    }

    energy = 0.0f;
    for (i = 0; i < pitch; ++i) {
        sum = 0.0f;
        for (j = 0; j < RT_LPC_ORDER; ++j) {
            sum += predictor[j] * g_afLpcExcitation[RT_LPC_ORDER + i - j - 1];
        }
        sum *= gain;
        g_afLpcFilteredExcitation[RT_LPC_ORDER + i] = sum + g_afLpcExcitation[RT_LPC_ORDER + i];
    }
    for (i = 0; i < pitch; ++i) {
        sum = 0.0f;
        for (j = 0; j < RT_LPC_ORDER; ++j) {
            sum += predictor[j] * g_afLpcFilteredExcitation[RT_LPC_ORDER + i - j - 1];
        }
        g_afLpcFilteredExcitation[RT_LPC_ORDER + i] =
            sum + g_afLpcFilteredExcitation[RT_LPC_ORDER + i];
        energy += g_afLpcFilteredExcitation[RT_LPC_ORDER + i] *
                  g_afLpcFilteredExcitation[RT_LPC_ORDER + i];
    }
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        g_afLpcExcitation[i] = g_afLpcExcitation[pitch + i];
        g_afLpcFilteredExcitation[i] = g_afLpcFilteredExcitation[pitch + i];
    }

    scale = (float)sqrt(rms * rms * (float)pitch / energy);
    for (i = 0; i < pitch; ++i) {
        output[i] = scale * g_afLpcFilteredExcitation[RT_LPC_ORDER + i];
    }
}

void rtLpc10Chanwr(int direction, int *pitchCode, int *rmsCode, int *rcCode, int *bits) {
    int i;
    int table[kLpcChanwrFields];

    if (direction == kLpcChanwrWrite) {
        table[0] = *pitchCode;
        table[1] = *rmsCode;
        table[2] = 0;
        for (i = 1; i <= RT_LPC_ORDER; ++i) {
            table[i + 2] = rcCode[RT_LPC_ORDER - i] & kLpcChanwrCodeMask;
        }
        for (i = 1; i <= kLpcChanwrBits; ++i) {
            bits[i - 1] = table[g_anLpcBitOrder[i - 1] - 1] & 1;
            table[g_anLpcBitOrder[i - 1] - 1] >>= 1;
        }
        bits[kLpcChanwrSyncBit] = g_nLpcSyncBit & 1;
        g_nLpcSyncBit = 1 - g_nLpcSyncBit;
    } else if (direction == kLpcChanwrRead) {
        for (i = 0; i < kLpcChanwrFields; ++i) {
            table[i] = 0;
        }
        // Yes, the binary reads each bit one entry after the entry the packing writes it to.
        for (i = 1; i <= kLpcChanwrBits; ++i) {
            table[g_anLpcBitOrder[kLpcChanwrBits - i] - 1] =
                table[g_anLpcBitOrder[kLpcChanwrBits - i] - 1] * 2 + bits[kLpcChanwrBits + 1 - i];
        }
        for (i = 1; i <= RT_LPC_ORDER; ++i) {
            if ((table[i + 2] & g_anLpcCodeSignBit[i - 1]) != 0) {
                table[i + 2] -= g_anLpcCodeSignBit[i - 1] << 1;
            }
        }
        *pitchCode = table[0];
        *rmsCode = table[1];
        for (i = 1; i <= RT_LPC_ORDER; ++i) {
            rcCode[i - 1] = table[kLpcChanwrFields - i];
        }
    }
}

void rtLpc10Deemp(float *samples, int count) {
    int i;
    float input;

    for (i = 0; i < count; ++i) {
        input = samples[i];
        samples[i] = (float)(input - g_fLpcDeemphasisInput1 * 0x1.fff2e48e8a71dp+0 +
                             g_fLpcDeemphasisInput2 + g_fLpcDeemphasisOutput1 * 2.75 -
                             g_fLpcDeemphasisOutput2 * 0x1.770a3d70a3d70p+1 +
                             g_fLpcDeemphasisOutput3 * 1.48 - g_fLpcDeemphasisOutput4 * 0.312);
        g_fLpcDeemphasisInput2 = g_fLpcDeemphasisInput1;
        g_fLpcDeemphasisInput1 = input;
        g_fLpcDeemphasisOutput4 = g_fLpcDeemphasisOutput3;
        g_fLpcDeemphasisOutput3 = g_fLpcDeemphasisOutput2;
        g_fLpcDeemphasisOutput2 = g_fLpcDeemphasisOutput1;
        g_fLpcDeemphasisOutput1 = samples[i];
    }
}

void rtLpc10Ham84(int input, int *output) {
    int parity;
    int entry;

    parity = input & kLpcHammingSymbolMask;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    parity &= 1;
    entry = g_anLpcHammingTable[input & kLpcHammingIndexMask];
    *output = entry & kLpcHammingValueMask;
    if ((entry & kLpcHammingValidBit) == 0 && parity == 0) {
        *output = -1;
    }
}

void rtLpc10Hp100(float *speech) {
    int i;
    float section;
    float output;

    for (i = 0; i < kLpcFrame; ++i) {
        section = speech[i] + g_fLpcHighPass1Z1 * 1.8590759f - g_fLpcHighPass1Z2 * 0.8648249f;
        output = section - g_fLpcHighPass1Z1 * 2.0f + g_fLpcHighPass1Z2;
        g_fLpcHighPass1Z2 = g_fLpcHighPass1Z1;
        g_fLpcHighPass1Z1 = section;
        section = output + g_fLpcHighPass2Z1 * 1.935715f - g_fLpcHighPass2Z2 * 0.9417004f;
        output = section - g_fLpcHighPass2Z1 * 2.0f + g_fLpcHighPass2Z2;
        g_fLpcHighPass2Z2 = g_fLpcHighPass2Z1;
        g_fLpcHighPass2Z1 = section;
        speech[i] = output * 0.902428f;
    }
}

void rtLpc10ResetState(void) {
    int i;
    int j;

    for (i = 0; i < kLpcExcitationSize; ++i) {
        g_afLpcExcitation[i] = 0.0f;
        g_afLpcFilteredExcitation[i] = 0.0f;
    }
    // Yes, the binary passes element counts as the byte counts of these clears.
    memset(g_aanLpcTrackPointer, 0, kLpcLags * kLpcTrackDepth);
    for (i = 0; i < kLpcLags; ++i) {
        g_afLpcTrackCost[i] = 0.0f;
    }
    memset(&g_anLpcPitchHistory[0], 0, RT_LPC_HISTORY);
    memset(&g_anLpcRmsHistory[0], 0, RT_LPC_HISTORY);
    memset(g_aanLpcRcHistory, 0, RT_LPC_HISTORY * RT_LPC_ORDER);
    memset(&g_anLpcEpochPitch[0], 0, RT_LPC_MAX_EPOCHS);
    memset(&g_anLpcEpochVoicing[0], 0, RT_LPC_MAX_EPOCHS);
    // Yes, the binary clears only the first RT_LPC_ORDER energies of the epochs.
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        g_afLpcEpochRms[i] = 0.0f;
        g_afLpcPredictor[i] = 0.0f;
    }
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        for (j = 0; j < RT_LPC_MAX_EPOCHS; ++j) {
            g_aafLpcEpochRc[i][j] = 0.0f;
        }
    }
}

void rtLpc10ResetAnalysis(void) {
    int i;

    // The binary biases each pointer back by the first index of its buffer; the analysis
    // subtracts that index instead.
    g_pfLpcInbuf = g_afLpcInbuf;
    g_pfLpcLpbuf = g_afLpcLpbuf;
    g_pfLpcIvbuf = g_afLpcIvbuf;
    g_pfLpcPebuf = g_afLpcPebuf;
    for (i = 0; i < kLpcSbufSize; ++i) {
        g_afLpcInbuf[i] = 0.0f;
        g_afLpcPebuf[i] = 0.0f;
    }
    for (i = 0; i < kLpcLbufSize; ++i) {
        g_afLpcLpbuf[i] = 0.0f;
    }
    for (i = 0; i < kLpcIvbufSize; ++i) {
        g_afLpcIvbuf[i] = 0.0f;
    }

    g_aanLpcVoicingWindow[kLpcWindowStart][kLpcCurrentFrame] = kLpcDefaultVoicingStart;
    g_aanLpcVoicingWindow[kLpcWindowEnd][kLpcCurrentFrame] = kLpcDefaultVoicingEnd;
    g_aanLpcVoicingWindow[kLpcWindowStart][1] = 0;
    g_aanLpcVoicingWindow[kLpcWindowEnd][1] = 0;
    g_aanLpcAnalysisWindow[kLpcWindowStart][kLpcCurrentFrame] = kLpcDefaultVoicingStart;
    g_aanLpcAnalysisWindow[kLpcWindowEnd][kLpcCurrentFrame] = kLpcDefaultVoicingEnd;
    g_aanLpcAnalysisWindow[kLpcWindowStart][1] = 0;
    g_aanLpcAnalysisWindow[kLpcWindowEnd][1] = 0;
    // Yes, the binary clears the reflection buffer as if it were ordered by frame, which clears
    // only its first 16 entries.
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        g_aafLpcRcBuffer[2 + i / kLpcAf][i % kLpcAf] = 0.0f;
        g_aafLpcRcBuffer[1 + i / kLpcAf][i % kLpcAf] = 0.0f;
        g_aafLpcRcBuffer[i / kLpcAf][i % kLpcAf] = 0.0f;
    }
    for (i = 0; i < kLpcAf; ++i) {
        g_afLpcRmsBuffer[i] = 0.0f;
    }
    for (i = 0; i < kLpcLags; ++i) {
        g_afLpcAmdf[i] = 0.0f;
    }
    // Yes, the binary passes the element count as the byte count.
    memset(g_aanLpcVoicing, 0, 2 * kLpcVoicingFrames);
    for (i = 0; i < kLpcUnusedStateSize; ++i) {
        g_anLpcUnusedState[i] = 0;
    }
}

// Interpolates the reflection coefficients and the energy of an epoch ending at @p used samples.
static inline void rtLpc10InterpolateEpoch(const float *rc,
                                           float rms,
                                           float epochRc[RT_LPC_ORDER][RT_LPC_MAX_EPOCHS],
                                           float *epochRms,
                                           int epoch,
                                           int used,
                                           int pitch,
                                           int length) {
    int j;
    float proportion;
    float oldLogArea;
    float newLogArea;
    float logArea;

    proportion = (float)((used - pitch * 0.5) / (float)length);
    for (j = 0; j < RT_LPC_ORDER; ++j) {
        oldLogArea =
            (float)log((g_afLpcSynthPreviousRc[j] + 1.0f) / (1.0f - g_afLpcSynthPreviousRc[j]));
        newLogArea = (float)log((rc[j] + 1.0f) / (1.0f - rc[j]));
        logArea = (float)exp(oldLogArea + proportion * (newLogArea - oldLogArea));
        epochRc[j][epoch] = (logArea - 1.0f) / (logArea + 1.0f);
    }
    epochRms[epoch] = (float)(log(g_fLpcSynthPreviousRms) +
                              proportion * (log(rms) - log(g_fLpcSynthPreviousRms)));
    epochRms[epoch] = (float)exp(epochRms[epoch]);
}

void rtLpc10Pitsyn(int *voice,
                   int *pitch,
                   float *rms,
                   float *rc,
                   int *epochVoicing,
                   int *epochPitch,
                   float *epochRms,
                   float epochRc[RT_LPC_ORDER][RT_LPC_MAX_EPOCHS],
                   int *epochs,
                   float *ratio) {
    int i;
    int j;
    int voicing;
    int length;
    int start;
    int used;
    int period;
    int unvoiced;
    float slope;
    float unvoicedPitch;
    float transition;

    if (*rms < 1.0f) {
        *rms = 1.0f;
    }
    if (g_fLpcSynthPreviousRms < 1.0f) {
        g_fLpcSynthPreviousRms = 1.0f;
    }
    unvoicedPitch = 0.0f;
    *ratio = (float)(*rms / (g_fLpcSynthPreviousRms + 8.0));

    if (g_wLpcSynthFirst != 0) {
        voicing = voice[1];
        if (voicing == 0) {
            *pitch = kLpcFrame / 4;
        }
        *epochs = kLpcFrame / *pitch;
        g_nLpcSynthCarry = kLpcFrame - *epochs * *pitch;
        for (i = 1; i <= *epochs; ++i) {
            for (j = 0; j < RT_LPC_ORDER; ++j) {
                epochRc[j][i - 1] = rc[j];
            }
            epochVoicing[i - 1] = voicing;
            epochPitch[i - 1] = *pitch;
            epochRms[i - 1] = *rms;
        }
        g_wLpcSynthFirst = 0;
    } else {
        transition = 0.0f;
        length = g_nLpcSynthCarry + kLpcFrame;
        *epochs = 0;
        used = 0;
        start = 1;
        if (voice[0] == g_nLpcSynthPreviousVoicing && voice[1] == voice[0]) {
            if (voice[1] == 0) {
                *pitch = kLpcFrame / 4;
                g_nLpcSynthPreviousPitch = kLpcFrame / 4;
                if (*ratio > 8.0f) {
                    g_fLpcSynthPreviousRms = *rms;
                }
            }
            slope = (float)(*pitch - g_nLpcSynthPreviousPitch) / (float)length;
            voicing = voice[1];
        } else if (g_nLpcSynthPreviousVoicing != 1) {
            if (g_nLpcSynthPreviousVoicing == voice[0]) {
                unvoiced = (int)(length - 45.0);
            } else {
                unvoiced = (int)(length - 135.0);
            }
            epochPitch[0] = (int)(unvoiced * 0.5);
            epochPitch[1] = unvoiced - epochPitch[0];
            epochVoicing[0] = 0;
            epochVoicing[1] = 0;
            epochRms[0] = g_fLpcSynthPreviousRms;
            epochRms[1] = g_fLpcSynthPreviousRms;
            for (i = 0; i < RT_LPC_ORDER; ++i) {
                epochRc[i][0] = g_afLpcSynthPreviousRc[i];
                epochRc[i][1] = g_afLpcSynthPreviousRc[i];
                g_afLpcSynthPreviousRc[i] = rc[i];
            }
            slope = 0.0f;
            *epochs = 2;
            g_nLpcSynthPreviousPitch = *pitch;
            used = unvoiced;
            start = unvoiced + 1;
            voicing = 1;
        } else {
            if (g_nLpcSynthPreviousVoicing != voice[0]) {
                length = (int)(g_nLpcSynthCarry + 45.0);
            } else {
                length = (int)(g_nLpcSynthCarry + 135.0);
            }
            for (i = 0; i < RT_LPC_ORDER; ++i) {
                g_afLpcSynthSavedRc[i] = rc[i];
                rc[i] = g_afLpcSynthPreviousRc[i];
            }
            voicing = 1;
            slope = 0.0f;
            transition = 1.0f;
        }

        for (;;) {
            for (i = start; i <= length; ++i) {
                period = (int)((float)g_nLpcSynthPreviousPitch + slope * (float)i + 0.5);
                if (unvoicedPitch != 0.0) {
                    period = (int)unvoicedPitch;
                }
                if (period <= i - used) {
                    ++*epochs;
                    if (*epochs > RT_LPC_MAX_EPOCHS) {
                        return; // Yes, the binary returns without saving the history.
                    }
                    epochPitch[*epochs - 1] = period;
                    *pitch = period;
                    epochVoicing[*epochs - 1] = voicing;
                    used += period;
                    rtLpc10InterpolateEpoch(
                        rc, *rms, epochRc, epochRms, *epochs - 1, used, period, length);
                }
            }
            if (transition != 1.0f) {
                break;
            }
            transition = 0.0f;
            start = used + 1;
            length = g_nLpcSynthCarry + kLpcFrame;
            slope = 0.0f;
            voicing = 0;
            unvoicedPitch = (float)((length - start) * 0.5);
            if (unvoicedPitch > 90.0f) {
                unvoicedPitch = (float)(unvoicedPitch * 0.5);
            }
            g_fLpcSynthPreviousRms = *rms;
            for (i = 0; i < RT_LPC_ORDER; ++i) {
                rc[i] = g_afLpcSynthSavedRc[i];
                g_afLpcSynthPreviousRc[i] = g_afLpcSynthSavedRc[i];
            }
        }
        g_nLpcSynthCarry = length - used;
    }

    if (*epochs != 0) {
        g_nLpcSynthPreviousVoicing = voice[1];
        g_nLpcSynthPreviousPitch = *pitch;
        g_fLpcSynthPreviousRms = *rms;
        for (i = 0; i < RT_LPC_ORDER; ++i) {
            g_afLpcSynthPreviousRc[i] = rc[i];
        }
    }
}

void rtLpc10Irc2pc(float epochRc[RT_LPC_ORDER][RT_LPC_MAX_EPOCHS],
                   float *predictor,
                   float *gain,
                   int epoch,
                   float gainScale) {
    int i;
    int j;
    float temp[RT_LPC_ORDER];

    *gain = 1.0f;
    for (i = 0; i < RT_LPC_ORDER; ++i) {
        *gain = (float)(*gain * (1.0 - epochRc[i][epoch] * epochRc[i][epoch]));
    }
    *gain = (float)(gainScale * sqrt(*gain));
    predictor[0] = epochRc[0][epoch];
    for (i = 1; i < RT_LPC_ORDER; ++i) {
        for (j = 0; j < i; ++j) {
            temp[j] = predictor[j] - epochRc[i][epoch] * predictor[i - j - 1];
        }
        for (j = 0; j < i; ++j) {
            predictor[j] = temp[j];
        }
        predictor[i] = epochRc[i][epoch];
    }
}

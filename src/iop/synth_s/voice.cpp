#include "synth_s/voice.h"

#include <libsd.h>

#include "synth_s/bank.h"
#include "synth_s/synth.h"

namespace {

constexpr unsigned char kNoChannel = 17;
constexpr unsigned char kNoNote = 128;
constexpr unsigned short kNoBank = 8;
constexpr unsigned char kFullVelocity = 127;
constexpr unsigned int kNewestAge = 0x7fffffff;

constexpr int kCentsPerSemitone = 100;
constexpr unsigned int kMaxPitch = 0x3fff;
constexpr unsigned int kOutputRate = 48000;
constexpr unsigned int kSurroundInvert = 0x7fff;

enum PanSide {
    kPanLeft,
    kPanRight,
    kNumPanSides,
};

// Left and right volume curve indices for each pan position.
// NTSC-U/C: 0x00009900
const unsigned char kPanTable[][kNumPanSides] = {
    {127, 0},  {126, 11}, {125, 15}, {125, 19}, {124, 22}, {124, 25}, {123, 27}, {123, 29},
    {122, 31}, {122, 33}, {121, 35}, {121, 37}, {120, 39}, {120, 40}, {119, 42}, {119, 43},
    {118, 45}, {118, 46}, {117, 47}, {117, 49}, {116, 50}, {116, 51}, {115, 52}, {114, 54},
    {114, 55}, {113, 56}, {113, 57}, {112, 58}, {112, 59}, {111, 60}, {110, 61}, {110, 62},
    {109, 63}, {109, 64}, {108, 65}, {108, 66}, {107, 67}, {106, 68}, {106, 69}, {105, 70},
    {105, 71}, {104, 72}, {103, 73}, {103, 73}, {102, 74}, {102, 75}, {101, 76}, {100, 77},
    {100, 78}, {99, 78},  {98, 79},  {98, 80},  {97, 81},  {96, 82},  {96, 82},  {95, 83},
    {94, 84},  {94, 85},  {93, 85},  {92, 86},  {92, 87},  {91, 88},  {90, 88},  {90, 89},
    {89, 90},  {88, 90},  {88, 91},  {87, 92},  {86, 92},  {85, 93},  {85, 94},  {84, 94},
    {83, 95},  {82, 96},  {82, 96},  {81, 97},  {80, 98},  {79, 98},  {78, 99},  {78, 100},
    {77, 100}, {76, 101}, {75, 102}, {74, 102}, {73, 103}, {73, 103}, {72, 104}, {71, 105},
    {70, 105}, {69, 106}, {68, 106}, {67, 107}, {66, 108}, {65, 108}, {64, 109}, {63, 109},
    {62, 110}, {61, 110}, {60, 111}, {59, 112}, {58, 112}, {57, 113}, {56, 113}, {55, 114},
    {54, 114}, {52, 115}, {51, 116}, {50, 116}, {49, 117}, {47, 117}, {46, 118}, {45, 118},
    {43, 119}, {42, 119}, {40, 120}, {39, 120}, {37, 121}, {35, 121}, {33, 122}, {31, 122},
    {29, 123}, {27, 123}, {25, 124}, {22, 124}, {19, 125}, {15, 125}, {11, 126}, {0, 127},
};

// Linear gain for each MIDI level, zero to 1023.
// NTSC-U/C: 0x00009a00
const unsigned short kVolumeTable[] = {
    0,   8,   16,  24,  32,  40,  48,  56,  64,  72,  81,  89,  97,  105,  113,  121,
    129, 137, 145, 153, 161, 169, 177, 185, 193, 201, 209, 217, 226, 234,  242,  250,
    258, 266, 274, 282, 290, 298, 306, 314, 322, 330, 338, 346, 354, 362,  371,  379,
    387, 395, 403, 411, 419, 427, 435, 443, 451, 459, 467, 475, 483, 491,  499,  507,
    516, 524, 532, 540, 548, 556, 564, 572, 580, 588, 596, 604, 612, 620,  628,  636,
    644, 652, 661, 669, 677, 685, 693, 701, 709, 717, 725, 733, 741, 749,  757,  765,
    773, 781, 789, 797, 806, 814, 822, 830, 838, 846, 854, 862, 870, 878,  886,  894,
    902, 910, 918, 926, 934, 942, 951, 959, 967, 975, 983, 991, 999, 1007, 1015, 1023,
};

} // namespace

void Voice::Init() {
    mChannel = kNoChannel;
    mNote = kNoNote;
    mBank = kNoBank;
    mVelocity = kFullVelocity;
    mState = kVoiceFree;
    mPriority = 0;
    mAge = kNewestAge;
    mSampleDesc = nullptr;
    mPitch = -1;
    mVibratoPhase = -1;
    mVibratoTable = -1;
}

void Voice::Dump() const {
    if (mSampleDesc != nullptr) {
        mSampleDesc->Dump();
    }
}

int Voice::FindFree(Voice *voices, int count, int *stealIndex) {
    int freeIndex = -1;
    *stealIndex = 0;
    if (count <= 0) {
        return freeIndex;
    }
    if (voices[0].mState == kVoiceFree) {
        freeIndex = 0;
        *stealIndex = -1;
        return freeIndex;
    }
    for (int i = 0;;) {
        const Voice &voice = voices[i];
        const Voice &candidate = voices[*stealIndex];
        if ((voice.mPriority < candidate.mPriority) && (voice.mState == kVoicePlaying)) {
            *stealIndex = i;
        } else if ((voice.mPriority == candidate.mPriority) && (voice.mAge < candidate.mAge) &&
                   (voice.mState == kVoicePlaying)) {
            *stealIndex = i;
        }
        ++i;
        if (i >= count) {
            break;
        }
        if (voices[i].mState == kVoiceFree) {
            freeIndex = i;
            *stealIndex = -1;
            break;
        }
    }
    return freeIndex;
}

void Voice::CalculateVolume(unsigned char velocity,
                            unsigned char bankVolume,
                            unsigned char programVolume,
                            unsigned char sampleVolume,
                            unsigned char channelVolume,
                            unsigned char channelExpression,
                            unsigned char pan,
                            unsigned int *left,
                            unsigned int *right,
                            signed char surround) {
    const unsigned int panLeft = kVolumeTable[kPanTable[pan][kPanLeft]];
    const unsigned int panRight = kVolumeTable[kPanTable[pan][kPanRight]];
    unsigned int gain =
        (kVolumeTable[velocity] * kVolumeTable[bankVolume] * kVolumeTable[programVolume]) >> 8;
    gain = (gain * kVolumeTable[sampleVolume]) >> 10;
    gain = (gain * kVolumeTable[channelVolume]) >> 10;
    gain = (gain * kVolumeTable[channelExpression]) >> 10;
    *left = (panLeft * gain) >> 18;
    *right = (panRight * gain) >> 18;
    if (surround != 0) {
        // Yes, retail compares the two output pointers to choose the side it inverts.
        if (right < left) {
            *right = kSurroundInvert - *right;
        } else {
            *left = kSurroundInvert - *left;
        }
    }
}

unsigned int Voice::CalculatePitch(
    unsigned char note, unsigned char baseKey, int semitones, int cents, int sampleRate) {
    if (cents < 0) {
        --semitones;
        cents += kCentsPerSemitone;
    }
    // Yes, retail passes the cents as the centre's fine tuning.
    unsigned int pitch = sceSdNote2Pitch(baseKey, cents, note + semitones, 0);
    if (pitch > kMaxPitch) {
        pitch = kMaxPitch;
    }
    return (pitch * sampleRate) / kOutputRate;
}

void Voice::ComputeVolume(unsigned int *left, unsigned int *right) const {
    const Channel &channel = Synth::sChannels[mChannel];
    Bank &bank = Bank::sBanks[channel.mBank];
    if (!Synth::ChannelHasProgram(mChannel)) {
        return;
    }
    const BankProgram *program = bank.GetProgram(channel.mProgram);
    const unsigned char pan = Synth::CombinePan(channel.mPan, program->mPan, mSampleDesc->mPan);
    CalculateVolume(mVelocity,
                    bank.mHeader.mVolume,
                    program->mVolume,
                    mSampleDesc->mVolume,
                    channel.mVolume,
                    channel.mExpression,
                    pan,
                    left,
                    right,
                    mSampleDesc->mSurround);
}

unsigned int Voice::ComputePitch() const {
    const Channel &channel = Synth::sChannels[mChannel];
    Bank &bank = Bank::sBanks[channel.mBank];
    (void)Synth::ChannelHasProgram(mChannel); // Yes, retail discards the check.
    const BankProgram *program = bank.GetProgram(channel.mProgram);
    return CalculatePitch(mNote,
                          mSampleDesc->mBaseKey,
                          channel.mPitchBendSemitones + program->mTranspose +
                              mSampleDesc->mTranspose + channel.mTranspose,
                          channel.mPitchBendCents + program->mFineTranspose +
                              mSampleDesc->mFineTranspose,
                          mSampleDesc->mSample->mSampleRate);
}

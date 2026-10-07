#include "synth/syntheffects.h"

#include "game/gamedb.h"
#include "os/debug.h"
#include "os/string.h"
#include "synth/synth.h"

namespace {

constexpr int kDefaultFreqCeiling = 10000;
constexpr int kDefaultFreqFloor = 500;
constexpr int kDefaultInitFreq = 1000;
constexpr float kDefaultResonance = 0.5f;

// The sweeping filters, the other values of SoftFxFilter::mType that read parameters.
constexpr int kTypeSweep2 = 2;
constexpr int kTypeSweep5 = 5;
constexpr int kTypeSweep7 = 7;
constexpr int kTypeSweep9 = 9;

// The delay ranges are given in milliseconds and stored in samples at 48 kHz.
constexpr float kSamplesPerMs = 48.0f;
constexpr float kDegreesPerCycle = 360.0f;

// The two values of a range entry.
constexpr int kRangeLow = 1;
constexpr int kRangeHigh = 2;

constexpr unsigned char kMidiControlChange = 0xb0;
constexpr unsigned char kControlMode = 0x4b;
constexpr unsigned char kControlDepthLeft = 0x4c;
constexpr unsigned char kControlDepthRight = 0x4d;
constexpr unsigned char kControlDelay = 0x4e;
constexpr unsigned char kControlFeedback = 0x4f;

// Pack a control change the way Synth::SendPackedMessage() takes it, the status byte lowest.
inline unsigned int PackControl(unsigned char nChannel, unsigned char nControl, int nValue) {
    constexpr int kControlShift = 8;
    constexpr int kValueShift = 16;
    return static_cast<unsigned char>(kMidiControlChange | nChannel) |
           (static_cast<unsigned int>(nControl) << kControlShift) |
           (static_cast<unsigned int>(static_cast<unsigned char>(nValue)) << kValueShift);
}

// Read the software effect from the `filter` entry of the first bus.
inline void LoadFilter(SoftFxFilter &filter, DataArray *pBus) {
    DataArray *pFilter = pBus->FindArray("filter", false);
    int nType = SoftFxFilter::kTypeOff;
    pFilter->FindInt("type", &nType, false);
    filter.mType = nType;
    switch (nType) {
    case kTypeSweep2:
    case kTypeSweep5:
    case kTypeSweep7:
    case kTypeSweep9:
        pFilter->FindFloat("resonance", &filter.mMix.mResonance, false);
        pFilter->FindInt("init_freq", &filter.mParam.mInitFreq, false);
        pFilter->FindInt("freq_floor", &filter.mFreqFloor, false);
        pFilter->FindInt("freq_ceiling", &filter.mFreqCeiling, false);
        break;
    case SoftFxFilter::kTypeDistort:
        pFilter->FindFloat("distortion", &filter.mParam.mDistortion, false);
        pFilter->FindFloat("feedback", &filter.mMix.mFeedback, false);
        break;
    case SoftFxFilter::kTypeChorus: {
        pFilter->FindFloat("feedback", &filter.mMix.mFeedback, true);
        pFilter->FindFloat("attenuation", &filter.mParam.mAttenuation, true);
        DataArray *pTaps = pFilter->FindArray("taps", false);
        for (int i = 0; i < SoftFxFilter::kNumTaps; ++i) {
            SoftFxFilter::Tap &tap = filter.mTaps[i];
            DataArray *pTap = pTaps->Array(i + 1);
            pTap->FindInt("LFO", &tap.mLfo, true);
            DataArray *pRange = pTap->FindArray("delay_range", true);
            const float fLow = pRange->Float(kRangeLow);
            const float fHigh = pRange->Float(kRangeHigh);
            tap.mDelayMax = static_cast<int>(fHigh * kSamplesPerMs);
            tap.mDelayMin = static_cast<int>(fLow * kSamplesPerMs);
            pRange = pTap->FindArray("osc_range", true);
            tap.mOscMin = pRange->Float(kRangeLow);
            tap.mOscMax = pRange->Float(kRangeHigh);
            int nPhase;
            pTap->FindInt("phase", &nPhase, true);
            tap.mPhase = static_cast<float>(nPhase) / kDegreesPerCycle;
            pTap->FindFloat("volume", &tap.mVolume, true);
        }
        break;
    }
    default:
        if (static_cast<unsigned int>(nType) >= SoftFxFilter::kTypeCount) {
            DebugWarn("unknown filter");
        }
        break;
    }
}

} // namespace

SynthEffects::SynthEffects() {
    mFilter.mType = SoftFxFilter::kTypeOff;
    mFilter.mFreqCeiling = kDefaultFreqCeiling;
    mFilter.mFreqFloor = kDefaultFreqFloor;
    mFilter.mReserved04 = 0;
    for (SoftFxFilter::Tap &tap : mFilter.mTaps) {
        tap = {};
    }
    mFilter.mParam.mInitFreq = kDefaultInitFreq;
    mFilter.mMix.mResonance = kDefaultResonance;
    mSoftFxOutput = 0;
    mBusToSoftFx = 0;
    mBusToCoreFx = 0;
}

SynthEffects::SynthEffects(DataArray *pConfig) : SynthEffects() {
    Load(pConfig);
}

void SynthEffects::Load(DataArray *pConfig) {
    for (int nBus = 1; nBus <= kNumBuses; ++nBus) {
        DataArray *pBus = nullptr;
        const int nRuleSet = TheGameDb->mRuleSet;
        if (nRuleSet == GameDb::kRuleSetRemix ||
            (nRuleSet == GameDb::kRuleSetGame && TheGameDb->mLoadRemix != 0)) {
            pBus = pConfig->FindArray(FormatString("remix_bus_%i", nBus), false);
        }
        if (pBus == nullptr) {
            pBus = pConfig->FindArray(FormatString("bus_%i", nBus), false);
            if (pBus == nullptr) {
                for (int i = 1; i < pConfig->mSize; ++i) {
                    DataArray *pEntry = pConfig->Array(i);
                    int nNumber = 0;
                    pEntry->FindInt("bus", &nNumber, false);
                    if (nNumber == nBus) {
                        pBus = pEntry;
                        break;
                    }
                }
            }
        }

        int nMode = 0;
        int nDepthRight = 0;
        int nDepthLeft = 0;
        int nDelay = 0;
        int nFeedback = 0;
        pBus->FindInt("mode", &nMode, false);
        pBus->FindInt("depth_right", &nDepthRight, false);
        pBus->FindInt("depth_left", &nDepthLeft, false);
        pBus->FindInt("delay", &nDelay, false);
        pBus->FindInt("feedback", &nFeedback, false);
        const unsigned char nChannel = static_cast<unsigned char>(nBus);
        mMessages.push_back(PackControl(nChannel, kControlMode, nMode));
        mMessages.push_back(PackControl(nChannel, kControlDepthLeft, nDepthLeft));
        mMessages.push_back(PackControl(nChannel, kControlDepthRight, nDepthRight));
        mMessages.push_back(PackControl(nChannel, kControlDelay, nDelay));
        mMessages.push_back(PackControl(nChannel, kControlFeedback, nFeedback));

        if (nBus == 1) {
            LoadFilter(mFilter, pBus);
            pBus->FindInt("soft_fx_output", &mSoftFxOutput, false);
            pBus->FindBool("fx_bus_to_softfx", &mBusToSoftFx, false);
            pBus->FindBool("fx_bus_to_corefx", &mBusToCoreFx, false);
        }
    }
}

void SynthEffects::Apply() {
    Synth *pSynth = TheSynth;
    (void)pSynth->DisableSoftFx(); // Yes, the binary discards the result.
    pSynth->SendMessages(mMessages);
    if (mFilter.mType == SoftFxFilter::kTypeOff) {
        pSynth->SetBusToSoftFx(0);
        pSynth->SetBusToCoreFx(1);
        pSynth->SetSoftFxFilter(mFilter);
    } else {
        (void)pSynth->EnableSoftFx(); // Yes, the binary discards the result.
        pSynth->SetSoftFxFilter(mFilter);
        pSynth->VirtualSlot32(mSoftFxOutput);
        pSynth->SetBusToSoftFx(mBusToSoftFx);
        pSynth->SetBusToCoreFx(mBusToCoreFx);
    }
}

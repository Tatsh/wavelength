#include "gs/midichannelstate.h"

#include <vector>

#include "gs/multimuse.h"

namespace {

constexpr int kUnset = -1;

// The high nibble of a status byte and the messages the state records.
constexpr unsigned char kStatusMask = 0xf0;
constexpr unsigned char kChannelMask = 0x0f;
constexpr unsigned char kStatusControlChange = 0xb0;
constexpr unsigned char kStatusProgramChange = 0xc0;
constexpr unsigned char kStatusChannelPressure = 0xd0;
constexpr unsigned char kStatusPitchBend = 0xe0;

// Controllers from 96 up are not part of the state.
constexpr unsigned char kFirstIgnoredController = 96;

} // namespace

MidiChannelState::MidiChannelState(unsigned char nChannel)
    : mChannel(nChannel), mProgram(kUnset), mPressure(kUnset), mBendLow(kUnset), mBendHigh(kUnset),
      mMuse(nullptr), mChanged(0) {
    Reset();
}

MidiChannelState::~MidiChannelState() {
}

void MidiChannelState::OnMessage(unsigned char nStatus,
                                 unsigned char nData1,
                                 unsigned char nData2) {
    const unsigned char nType = nStatus & kStatusMask;
    if (nType != kStatusProgramChange && nType != kStatusChannelPressure &&
        nType != kStatusPitchBend && nType != kStatusControlChange) {
        return;
    }
    if ((nStatus & kChannelMask) != mChannel) {
        return;
    }
    switch (nType) {
    case kStatusProgramChange: {
        const bool bChanged = mProgram != nData1;
        mProgram = nData1;
        mChanged = mChanged != 0 || bChanged;
        break;
    }
    case kStatusChannelPressure: {
        const bool bChanged = mPressure != nData1;
        mPressure = nData1;
        mChanged = mChanged != 0 || bChanged;
        break;
    }
    case kStatusPitchBend: {
        const bool bChanged = mBendLow != nData1 || mBendHigh != nData2;
        mBendHigh = nData2;
        mChanged = mChanged != 0 || bChanged;
        mBendLow = nData1;
        break;
    }
    case kStatusControlChange:
        if (nData1 < kFirstIgnoredController) {
            int &nValue = mControllers[nData1];
            mChanged = mChanged != 0 || nValue != nData2;
            nValue = nData2;
        }
        break;
    default:
        break;
    }
}

StdMidiMuse *MidiChannelState::GetProgramMuse() {
    if (mProgram == kUnset) {
        return nullptr;
    }
    if (mProgramMuse.Get() == nullptr || mProgramMuse->mData1 != mProgram) {
        mProgramMuse = Ptr<StdMidiMuse>(
            StdMidiMuse::NewProgramChange(mChannel, static_cast<unsigned char>(mProgram)));
    }
    return mProgramMuse.Get();
}

StdMidiMuse *MidiChannelState::GetPressureMuse() {
    if (mPressure == kUnset) {
        return nullptr;
    }
    if (mPressureMuse.Get() == nullptr || mPressureMuse->mData1 != mPressure) {
        mPressureMuse = Ptr<StdMidiMuse>(
            StdMidiMuse::NewChannelPressure(mChannel, static_cast<unsigned char>(mPressure)));
    }
    return mPressureMuse.Get();
}

StdMidiMuse *MidiChannelState::GetPitchBendMuse() {
    if (mBendLow == kUnset) {
        return nullptr;
    }
    if (mBendMuse.Get() == nullptr || mBendMuse->mData1 != mBendLow ||
        mBendMuse->mData2 != mBendHigh) {
        mBendMuse = Ptr<StdMidiMuse>(new StdMidiMuse(mChannel | kStatusPitchBend,
                                                     static_cast<unsigned char>(mBendLow),
                                                     static_cast<unsigned char>(mBendHigh)));
    }
    return mBendMuse.Get();
}

StdMidiMuse *MidiChannelState::GetControllerMuse(unsigned char nController) {
    const int nValue = mControllers[nController];
    if (nValue == kUnset) {
        return nullptr;
    }
    Ptr<StdMidiMuse> &muse = mControllerMuses[nController];
    if (muse.Get() == nullptr || muse->mData2 != nValue) {
        muse = Ptr<StdMidiMuse>(StdMidiMuse::NewControlChange(
            mChannel, nController, static_cast<unsigned char>(nValue)));
    }
    return muse.Get();
}

Muse *MidiChannelState::GetMuse() {
    if (!mChanged) {
        return mMuse.Get();
    }
    std::vector<Muse *> muses;
    mChanged = 0;
    for (int i = 0; i < kNumControllers; ++i) {
        if (StdMidiMuse *pMuse = GetControllerMuse(static_cast<unsigned char>(i))) {
            muses.push_back(pMuse);
        }
    }
    if (StdMidiMuse *pMuse = GetProgramMuse()) {
        muses.push_back(pMuse);
    }
    if (StdMidiMuse *pMuse = GetPressureMuse()) {
        muses.push_back(pMuse);
    }
    if (StdMidiMuse *pMuse = GetPitchBendMuse()) {
        muses.push_back(pMuse);
    }
    if (muses.empty()) {
        mMuse = Ptr<Muse>(nullptr);
    } else if (muses.size() == 1) {
        mMuse = Ptr<Muse>(muses[0]);
    } else {
        MultiMuse *pMulti = new MultiMuse(0);
        for (Muse *pMuse : muses) {
            pMulti->Add(pMuse, 0);
        }
        mMuse = Ptr<Muse>(pMulti);
    }
    return mMuse.Get();
}

void MidiChannelState::Reset() {
    mBendHigh = kUnset;
    mProgram = kUnset;
    mPressure = kUnset;
    mBendLow = kUnset;
    for (int i = kNumControllers - 1; i >= 0; --i) {
        mControllers[i] = kUnset;
    }
    mProgramMuse = Ptr<StdMidiMuse>(nullptr);
    mPressureMuse = Ptr<StdMidiMuse>(nullptr);
    mBendMuse = Ptr<StdMidiMuse>(nullptr);
    for (Ptr<StdMidiMuse> &muse : mControllerMuses) {
        muse = Ptr<StdMidiMuse>(nullptr);
    }
    mMuse = Ptr<Muse>(nullptr);
    mChanged = 0;
}

void MidiChannelState::SetChannel(unsigned char nChannel) {
    Reset();
    mChannel = nChannel;
}

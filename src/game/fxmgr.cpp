#include "game/fxmgr.h"

#include "synth/synth.h"

namespace {

constexpr unsigned int kStatusControlChange = 0xb0;
constexpr unsigned int kByteMask = 0xff;
constexpr int kControllerShift = 8;
constexpr int kValueShift = 16;

// The general purpose controllers the filter value goes to.
constexpr unsigned int kFilterController1 = 18;
constexpr unsigned int kFilterController2 = 19;

// The effect depth controllers the chorus switches.
constexpr unsigned int kChorusController1 = 93;
constexpr unsigned int kChorusController2 = 94;
constexpr int kChorusOnValue = 2;
constexpr int kChorusOffValue = 0;

constexpr int kFirstSetBar = 0;
constexpr int kFirstSet = 0;
constexpr int kSetCountBytes = 1;
constexpr int kEffectBytes = 1;

unsigned int PackControlChange(int nChannel, unsigned int nController, int nValue) {
    return (kStatusControlChange | (static_cast<unsigned int>(nChannel) & kByteMask)) |
           (nController << kControllerShift) |
           ((static_cast<unsigned int>(nValue) & kByteMask) << kValueShift);
}

} // namespace

// Yes, the binary leaves mActive unset until Activate() or Reset().
FXMgr::FXMgr(int nNumChannels, int nChorusDepth)
    : mNumChannels(nNumChannels), mChorusDepth(nChorusDepth), mCurrentSet(kFirstSet) {
    AddSet(kFirstSetBar);
}

FXMgr::~FXMgr() {
}

void FXMgr::Activate() {
    mActive = 1;
    ApplySet(mCurrentSet);
}

void FXMgr::Reset() {
    mActive = 0;
    for (int nChannel = 0; nChannel < mNumChannels; ++nChannel) {
        ApplyFilter(nChannel, 0);
        ApplyStutter(nChannel, 0);
        ApplyChorus(nChannel, 0);
    }
}

void FXMgr::AddSet(int nBar) {
    const FXSet set{nBar, std::vector<FX>(mNumChannels, FX{0, 0, 0})};
    mSets.push_back(set);
}

void FXMgr::ApplySet(int nSet) {
    mCurrentSet = nSet;
    if (mActive == 0) {
        return;
    }
    const FXSet &set = mSets[nSet];
    for (int nChannel = 0; nChannel < mNumChannels; ++nChannel) {
        const FX &fx = set.mFX[nChannel];
        ApplyFilter(nChannel, fx.mFilter);
        ApplyStutter(nChannel, fx.mStutter);
        ApplyChorus(nChannel, fx.mChorus);
    }
}

int FXMgr::GetNumSets() const {
    return static_cast<int>(mSets.size());
}

int FXMgr::GetCurrentSet() const {
    return mCurrentSet;
}

void FXMgr::CopySet(int nFrom, int nTo) {
    mSets[nTo].mFX = mSets[nFrom].mFX;
}

int FXMgr::GetSetBar(int nSet) const {
    return mSets[nSet].mBar;
}

void FXMgr::SetStutter(int nSet, int nChannel, int nStutter) {
    mSets[nSet].mFX[nChannel].mStutter = nStutter;
    // Yes, the binary applies the change while the manager is not active as well.
    if (nSet == mCurrentSet) {
        ApplyStutter(nChannel, nStutter);
    }
}

void FXMgr::SetFilter(int nSet, int nChannel, int nFilter) {
    mSets[nSet].mFX[nChannel].mFilter = nFilter;
    if (nSet == mCurrentSet) {
        ApplyFilter(nChannel, nFilter);
    }
}

void FXMgr::SetChorus(int nSet, int nChannel, int nChorus) {
    mSets[nSet].mFX[nChannel].mChorus = nChorus;
    if (nSet == mCurrentSet) {
        ApplyChorus(nChannel, nChorus);
    }
}

int FXMgr::GetStutter(int nSet, int nChannel) const {
    return mSets[nSet].mFX[nChannel].mStutter;
}

int FXMgr::GetFilter(int nSet, int nChannel) const {
    return mSets[nSet].mFX[nChannel].mFilter;
}

int FXMgr::GetChorus(int nSet, int nChannel) const {
    return mSets[nSet].mFX[nChannel].mChorus;
}

void FXMgr::Load(BinStream &stream) {
    unsigned char nNumSets = 0;
    stream.Read(&nNumSets, kSetCountBytes);
    mSets.clear();
    for (int nSet = 0; nSet < nNumSets; ++nSet) {
        int nBar;
        stream.ReadEndian(&nBar, sizeof(nBar));
        const FXSet set{nBar, std::vector<FX>(mNumChannels, FX{0, 0, 0})};
        mSets.push_back(set);
        FXSet &added = mSets.back();
        for (int nChannel = 0; nChannel < mNumChannels; ++nChannel) {
            FX &fx = added.mFX[nChannel];
            unsigned char nByte;
            stream.Read(&nByte, kEffectBytes);
            fx.mStutter = nByte != 0;
            stream.Read(&nByte, kEffectBytes);
            fx.mFilter = nByte != 0;
            stream.Read(&nByte, kEffectBytes);
            fx.mChorus = nByte != 0;
        }
    }
    mCurrentSet = kFirstSet;
}

void FXMgr::Save(BinStream &stream) const {
    const unsigned char nNumSets = static_cast<unsigned char>(mSets.size());
    stream.Write(&nNumSets, kSetCountBytes);
    for (unsigned int nSet = 0; nSet < mSets.size(); ++nSet) {
        const FXSet &set = mSets[nSet];
        const int nBar = set.mBar;
        stream.WriteEndian(&nBar, sizeof(nBar));
        for (int nChannel = 0; nChannel < mNumChannels; ++nChannel) {
            const FX &fx = set.mFX[nChannel];
            unsigned char nByte = static_cast<unsigned char>(fx.mStutter);
            stream.Write(&nByte, kEffectBytes);
            nByte = static_cast<unsigned char>(fx.mFilter);
            stream.Write(&nByte, kEffectBytes);
            nByte = static_cast<unsigned char>(fx.mChorus);
            stream.Write(&nByte, kEffectBytes);
        }
    }
}

void FXMgr::ApplyFilter(int nChannel, int nFilter) {
    TheSynth->SendPackedMessage(PackControlChange(nChannel, kFilterController1, nFilter));
    TheSynth->SendPackedMessage(PackControlChange(nChannel, kFilterController2, nFilter));
}

void FXMgr::ApplyStutter(int nChannel, int nStutter) {
    mStutter.SetChannel(nChannel, nStutter);
}

void FXMgr::ApplyChorus(int nChannel, int nChorus) {
    TheSynth->SendPackedMessage(PackControlChange(
        nChannel, kChorusController1, (nChorus != 0) ? kChorusOnValue : kChorusOffValue));
    TheSynth->SendPackedMessage(
        PackControlChange(nChannel, kChorusController2, (nChorus != 0) ? mChorusDepth : 0));
}

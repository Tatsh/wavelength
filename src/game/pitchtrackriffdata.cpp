#include "game/pitchtrackriffdata.h"

#include <algorithm>

namespace {

constexpr int kNotFound = -1;

} // namespace

PitchTrackRiffData::PitchTrackRiffData() : mRiffSets(), mReserved10(1), mEnabled(true), mName() {
}

PitchTrackRiffData::~PitchTrackRiffData() {
}

void PitchTrackRiffData::AddRiffs(int nTick, Muse *pRiff0, Muse *pRiff1, Muse *pRiff2) {
    const TickObj<RiffSet> entry = MakeEntry(nTick, pRiff0, pRiff1, pRiff2);
    if (mRiffSets.empty() || mRiffSets.back().mPosition.mTick < nTick) {
        mRiffSets.push_back(entry);
        return;
    }
    mRiffSets.insert(std::lower_bound(mRiffSets.begin(), mRiffSets.end(), entry, TickBefore),
                     entry);
}

void PitchTrackRiffData::GetRiffs(int nIndex,
                                  Muse **ppRiff0,
                                  Muse **ppRiff1,
                                  Muse **ppRiff2) const {
    const RiffSet &riffs = mRiffSets[nIndex].mValue;
    *ppRiff0 = riffs.mRiffs[0].Get();
    *ppRiff1 = riffs.mRiffs[1].Get();
    *ppRiff2 = riffs.mRiffs[2].Get();
}

Muse *PitchTrackRiffData::GetRiff(int nTick, int nSlot) const {
    const int nIndex = FindRiffs(nTick);
    if (nIndex == kNotFound) {
        return nullptr;
    }
    const RiffSet &riffs = mRiffSets[nIndex].mValue;
    switch (nSlot) {
    case 0:
        return riffs.mRiffs[0].Get();
    case 1:
        return riffs.mRiffs[1].Get();
    case 2:
        return riffs.mRiffs[2].Get();
    default:
        return nullptr;
    }
}

int PitchTrackRiffData::FindRiffs(int nTick) const {
    const TickObj<RiffSet> key = MakeEntry(nTick, nullptr, nullptr, nullptr);
    const auto it = std::upper_bound(mRiffSets.begin(), mRiffSets.end(), key, TickBefore);
    if (it == mRiffSets.begin()) {
        return kNotFound;
    }
    return static_cast<int>((it - 1) - mRiffSets.begin());
}

void PitchTrackRiffData::SetEnabled(bool bEnabled) {
    mEnabled = bEnabled;
}

bool PitchTrackRiffData::IsEnabled() const {
    return mEnabled;
}

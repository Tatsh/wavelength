#include "game/catchtrackdata.h"

#include <algorithm>

namespace {

constexpr int kReservedGems = 500;
constexpr int kNoLength = -1;
constexpr int kNoGem = -1;
constexpr int kKeyLane = 0;

// Order gems by tick, then by lane.
bool GemBefore(const Gem &first, const Gem &second) {
    return first.mTick < second.mTick ||
           (first.mTick == second.mTick && first.mLane < second.mLane);
}

} // namespace

CatchTrackData::CatchTrackData(int nLengthTicks) : mLengthTicks(nLengthTicks) {
    mGems.reserve(kReservedGems);
}

CatchTrackData::CatchTrackData() : mLengthTicks(kNoLength) {
    mGems.reserve(kReservedGems);
}

CatchTrackData::~CatchTrackData() {
}

void CatchTrackData::AddGem(const Gem &gem) {
    if (mLengthTicks >= 0 && gem.mTick >= mLengthTicks) {
        return;
    }
    if (mGems.empty() || GemBefore(mGems.back(), gem)) {
        mGems.push_back(gem);
        return;
    }
    mGems.insert(std::lower_bound(mGems.begin(), mGems.end(), gem, GemBefore), gem);
}

void CatchTrackData::EraseSpan(PlayMap *pPlayMap, int nStartTick, int nEndTick) {
    const int nStart = pPlayMap->MapTick(nStartTick);
    const int nEnd = pPlayMap->MapTick(nEndTick);
    CheckGems();
    const Gem startKey{kKeyLane, nStart, Ptr<Muse>()};
    const auto itStart = std::lower_bound(mGems.begin(), mGems.end(), startKey, GemBefore);
    const Gem endKey{kKeyLane, nEnd, Ptr<Muse>()};
    const auto itEnd = std::lower_bound(mGems.begin(), mGems.end(), endKey, GemBefore);
    if (nStart < nEnd) {
        mGems.erase(itStart, itEnd);
    } else if (nEnd < nStart) {
        mGems.erase(itStart, mGems.end());
        mGems.erase(mGems.begin(), itEnd);
    }
    CheckGems();
}

int CatchTrackData::GetNumGems() const {
    return static_cast<int>(mGems.size());
}

Gem *CatchTrackData::GetGem(int nIndex) {
    return &mGems[nIndex];
}

const Gem *CatchTrackData::GetGem(int nIndex) const {
    return &mGems[nIndex];
}

int CatchTrackData::FindGemAt(int nTick) const {
    const Gem key{kKeyLane, nTick, Ptr<Muse>()};
    const auto it = std::lower_bound(mGems.begin(), mGems.end(), key, GemBefore);
    if (it == mGems.end() || it->mTick != nTick) {
        return kNoGem;
    }
    return static_cast<int>(it - mGems.begin());
}

int CatchTrackData::FindGem(int nTick) const {
    const Gem key{kKeyLane, nTick, Ptr<Muse>()};
    const auto it = std::lower_bound(mGems.begin(), mGems.end(), key, GemBefore);
    if (it == mGems.end()) {
        return kNoGem;
    }
    return static_cast<int>(it - mGems.begin());
}

void CatchTrackData::CheckGems() const {
    for (auto it = mGems.begin(); it != mGems.end(); ++it) {
    }
}

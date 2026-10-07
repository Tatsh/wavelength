#include "game/axetrackdata.h"

#include "game/tickobjvector.h"

namespace {

// The value the lookups return when no set follows.
constexpr int kNoNextSet = -1;

} // namespace

AxeTrackData::AxeTrackData(int nEndTick, int nReserved) : mEndTick(nEndTick), mReserved(nReserved) {
}

AxeTrackData::~AxeTrackData() {
    for (const auto &entry : mContours) {
        for (auto *pContour : entry.mValue.mContours) {
            delete pContour;
        }
    }
    for (const auto &entry : mHarmonies) {
        delete entry.mValue;
    }
}

void AxeTrackData::AddHarmony(int nTick, const AxeHarmony *pHarmony) {
    if (nTick >= mEndTick) {
        delete pHarmony;
        return;
    }
    TickObj<const AxeHarmony *> entry;
    entry.mPosition.mTick = nTick;
    entry.mValue = pHarmony;
    InsertSorted(mHarmonies, entry);
}

void AxeTrackData::AddContours(int nTick,
                               AxeContour *pFirst,
                               AxeContour *pSecond,
                               AxeContour *pThird) {
    if (nTick >= mEndTick) {
        delete pFirst;
        delete pSecond;
        delete pThird;
        return;
    }
    TickObj<ContourSet> entry;
    entry.mPosition.mTick = nTick;
    entry.mValue.mContours[0] = pFirst;
    entry.mValue.mContours[1] = pSecond;
    entry.mValue.mContours[2] = pThird;
    InsertSorted(mContours, entry);
}

int AxeTrackData::GetContours(int nTick,
                              AxeContour **ppFirst,
                              AxeContour **ppSecond,
                              AxeContour **ppThird) const {
    auto it = FindAtOrBefore(mContours, nTick);
    *ppFirst = it->mValue.mContours[0];
    *ppSecond = it->mValue.mContours[1];
    *ppThird = it->mValue.mContours[2];
    ++it;
    if (it == mContours.end()) {
        return kNoNextSet;
    }
    return it->mPosition.mTick;
}

int AxeTrackData::GetHarmony(int nTick, const AxeHarmony **ppHarmony) const {
    auto it = FindAtOrBefore(mHarmonies, nTick);
    *ppHarmony = it->mValue;
    ++it;
    if (it == mHarmonies.end()) {
        return kNoNextSet;
    }
    return it->mPosition.mTick;
}

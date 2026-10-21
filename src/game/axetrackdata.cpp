#include "game/axetrackdata.h"

#include <algorithm>
#include <utility>

namespace {

// The value the lookups return when no set follows.
constexpr int kNoNextSet = -1;

// Insert an entry in tick order, replicating AddOrdered without copying the stored value.
template <typename T>
void AddOwnedOrdered(TickObj<T> &&entry, std::vector<TickObj<T>> &values) {
    if (!values.empty() && entry.mPosition < values.back().mPosition) {
        const auto it = std::lower_bound(
            values.begin(), values.end(), entry.mPosition, [](const TickObj<T> &value, int nTick) {
                return value.mPosition < nTick;
            });
        values.insert(it, std::move(entry));
        return;
    }
    values.push_back(std::move(entry));
}

// Find the last entry at or before a tick, or the end when every entry follows it.
template <typename T>
typename std::vector<TickObj<T>>::const_iterator
FindOwnedAtOrBefore(const std::vector<TickObj<T>> &values, int nTick) {
    auto it = values.end();
    for (auto current = values.begin(); current != values.end(); ++current) {
        if (current->mPosition > nTick) {
            break;
        }
        it = current;
    }
    return it;
}

} // namespace

AxeTrackData::AxeTrackData(int nEndTick, int nReserved) : mEndTick(nEndTick), mReserved(nReserved) {
}

AxeTrackData::~AxeTrackData() {
    // Preserve the original order: every set of notes in order, then every chord. Member
    // destruction would destroy the chords first, because mHarmonies is declared last.
    for (auto &entry : mContours) {
        for (auto &pContour : entry.mValue.mContours) {
            pContour.reset();
        }
    }
    for (auto &entry : mHarmonies) {
        entry.mValue.reset();
    }
}

void AxeTrackData::AddHarmony(int nTick, const AxeHarmony *pHarmony) {
    std::unique_ptr<const AxeHarmony> pOwned(pHarmony);
    if (nTick >= mEndTick) {
        return;
    }
    TickObj<std::unique_ptr<const AxeHarmony>> entry;
    entry.mPosition = nTick;
    entry.mValue = std::move(pOwned);
    AddOwnedOrdered(std::move(entry), mHarmonies);
}

void AxeTrackData::AddContours(int nTick,
                               AxeContour *pFirst,
                               AxeContour *pSecond,
                               AxeContour *pThird) {
    TickObj<ContourSet> entry;
    entry.mPosition = nTick;
    entry.mValue.mContours[0].reset(pFirst);
    entry.mValue.mContours[1].reset(pSecond);
    entry.mValue.mContours[2].reset(pThird);
    if (nTick >= mEndTick) {
        // Preserve the original deletion order of a discarded set.
        entry.mValue.mContours[0].reset();
        entry.mValue.mContours[1].reset();
        entry.mValue.mContours[2].reset();
        return;
    }
    AddOwnedOrdered(std::move(entry), mContours);
}

int AxeTrackData::GetContours(int nTick,
                              AxeContour **ppFirst,
                              AxeContour **ppSecond,
                              AxeContour **ppThird) const {
    auto it = FindOwnedAtOrBefore(mContours, nTick);
    *ppFirst = it->mValue.mContours[0].get();
    *ppSecond = it->mValue.mContours[1].get();
    *ppThird = it->mValue.mContours[2].get();
    ++it;
    if (it == mContours.end()) {
        return kNoNextSet;
    }
    return it->mPosition;
}

int AxeTrackData::GetHarmony(int nTick, const AxeHarmony **ppHarmony) const {
    auto it = FindOwnedAtOrBefore(mHarmonies, nTick);
    *ppHarmony = it->mValue.get();
    ++it;
    if (it == mHarmonies.end()) {
        return kNoNextSet;
    }
    return it->mPosition;
}

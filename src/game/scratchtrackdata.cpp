#include "game/scratchtrackdata.h"

#include <algorithm>
#include <utility>

namespace {

// The value GetScratchers() returns when no set follows.
constexpr int kNoNextSet = -1;

} // namespace

ScratchTrackData::ScratchTrackData(int nEndTick) : mEndTick(nEndTick) {
}

ScratchTrackData::~ScratchTrackData() {
    // Preserve the original order, each set in order and each scratcher of a set in order.
    for (auto &entry : mSets) {
        for (auto &pScratcher : entry.mValue.mScratchers) {
            pScratcher.reset();
        }
    }
}

void ScratchTrackData::SetScratcher(int nTick, int nIndex, Scratcher *pScratcher) {
    std::unique_ptr<Scratcher> pOwned(pScratcher);
    if (nTick >= mEndTick) {
        return;
    }

    const auto upper = std::upper_bound(
        mSets.begin(), mSets.end(), nTick, [](int nValue, const TickObj<ScratcherSet> &entry) {
            return nValue < entry.mPosition;
        });
    auto it = (upper == mSets.begin()) ? mSets.end() : (upper - 1);
    if ((it != mSets.end()) && (it->mPosition == nTick)) {
        it->mValue.mScratchers[nIndex] = std::move(pOwned);
        return;
    }

    TickObj<ScratcherSet> entry;
    entry.mPosition = nTick;
    entry.mValue.mScratchers[nIndex] = std::move(pOwned);
    if (!mSets.empty() && !(entry.mPosition < mSets.back().mPosition)) {
        mSets.push_back(std::move(entry));
        return;
    }
    const auto insertAt = std::upper_bound(
        mSets.begin(),
        mSets.end(),
        entry,
        [](const TickObj<ScratcherSet> &first, const TickObj<ScratcherSet> &second) {
            return first.mPosition < second.mPosition;
        });
    mSets.insert(insertAt, std::move(entry));
}

int ScratchTrackData::GetScratchers(int nTick,
                                    Scratcher **ppFirst,
                                    Scratcher **ppSecond,
                                    Scratcher **ppThird) const {
    auto it = mSets.end();
    for (auto current = mSets.begin(); current != mSets.end(); ++current) {
        if (current->mPosition > nTick) {
            break;
        }
        it = current;
    }
    *ppFirst = it->mValue.mScratchers[0].get();
    *ppSecond = it->mValue.mScratchers[1].get();
    *ppThird = it->mValue.mScratchers[2].get();
    ++it;
    if (it == mSets.end()) {
        return kNoNextSet;
    }
    return it->mPosition;
}

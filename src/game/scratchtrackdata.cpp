#include "game/scratchtrackdata.h"

#include "game/tickobjvector.h"

namespace {

// The value GetScratchers() returns when no set follows.
constexpr int kNoNextSet = -1;

} // namespace

ScratchTrackData::ScratchTrackData(int nEndTick) : mEndTick(nEndTick) {
}

ScratchTrackData::~ScratchTrackData() {
    for (unsigned int i = 0; i < mSets.size(); ++i) {
        for (int j = 0; j < kSetSize; ++j) {
            delete mSets[i].mValue.mScratchers[j];
        }
    }
}

void ScratchTrackData::SetScratcher(int nTick, int nIndex, Scratcher *pScratcher) {
    if (nTick >= mEndTick) {
        delete pScratcher;
        return;
    }

    TickObj<ScratcherSet> entry;
    entry.mPosition.mTick = nTick;
    entry.mValue = {};
    auto it = UpperBoundByEntry(mSets, entry);
    it = (it == mSets.begin()) ? mSets.end() : (it - 1);
    if ((it != mSets.end()) && (it->mPosition.mTick == nTick)) {
        it->mValue.mScratchers[nIndex] = pScratcher;
        return;
    }

    entry.mValue.mScratchers[nIndex] = pScratcher;
    InsertSorted(mSets, entry);
}

int ScratchTrackData::GetScratchers(int nTick,
                                    Scratcher **ppFirst,
                                    Scratcher **ppSecond,
                                    Scratcher **ppThird) const {
    auto it = FindAtOrBefore(mSets, nTick);
    *ppFirst = it->mValue.mScratchers[0];
    *ppSecond = it->mValue.mScratchers[1];
    *ppThird = it->mValue.mScratchers[2];
    ++it;
    if (it == mSets.end()) {
        return kNoNextSet;
    }
    return it->mPosition.mTick;
}

#include "game/playmap.h"

#include <algorithm>

namespace {

// The length of a loop that repeats without end, and the end bar while one plays.
constexpr int kEndlessBars = 0x20000001;

} // namespace

PlayMap::PlayMap(int nTicksPerBar, int nEndBar)
    : mTicksPerBar(nTicksPerBar), mLoops(), mEndBar(nEndBar), mLooping(false) {
    mLoops.push_back(Loop{0, kEndlessBars, 0});
}

void PlayMap::AddLoop(int nStartBar, int nNumBars, int nPlayBar) {
    mLooping = true;
    const Loop &last = mLoops.back();
    const int nPartBars = (nPlayBar - last.mPlayBar) % last.mNumBars;
    if (nPartBars != 0) {
        mLoops.push_back(Loop{last.mStartBar, nPartBars, nPlayBar - nPartBars});
    }
    mLoops.push_back(Loop{nStartBar, nNumBars, nPlayBar});
}

int PlayMap::MapBar(int nBar) const {
    const Loop key{0, 0, nBar};
    auto it = std::upper_bound(mLoops.begin(), mLoops.end(), key);
    if (it != mLoops.begin()) {
        --it;
    }
    return it->mStartBar + (nBar - it->mPlayBar) % it->mNumBars;
}

int PlayMap::MapTick(int nTick) const {
    return MapBar(nTick / mTicksPerBar) * mTicksPerBar + nTick % mTicksPerBar;
}

void PlayMap::GetSegment(
    int nTick, int *pChangeTick, int *pEnd, int *pNextStart, int *pNextLength, int *pIsLast) {
    int nBar = nTick / mTicksPerBar;
    if (nBar < 0) {
        nBar = 0;
    }
    const Loop key{0, 0, nBar};
    auto it = std::upper_bound(mLoops.begin(), mLoops.end(), key);
    if (it != mLoops.begin()) {
        --it;
    }
    if (it->mNumBars == kEndlessBars) {
        *pChangeTick = kEndlessBars;
        *pIsLast = 1;
        return;
    }

    int nChangeBar = nBar - (nBar - it->mPlayBar) % it->mNumBars + it->mNumBars;
    *pEnd = (it->mStartBar + it->mNumBars) * mTicksPerBar;
    const auto next = it + 1;
    *pIsLast = next == mLoops.end();
    auto nextLoop = it;
    if (next != mLoops.end() && !(nChangeBar < next->mPlayBar)) {
        nextLoop = next;
        nChangeBar = next->mPlayBar;
    }
    *pNextStart = nextLoop->mStartBar * mTicksPerBar;
    *pNextLength = nextLoop->mNumBars * mTicksPerBar;
    *pChangeTick = nChangeBar * mTicksPerBar;
}

int PlayMap::GetEndBar() const {
    return mLooping ? kEndlessBars : mEndBar;
}

void PlayMap::ClearLoops() {
    mLoops.clear();
    mLoops.push_back(Loop{0, kEndlessBars, 0});
}

#include "game/playmap.h"

#include <algorithm>

namespace {

// The length of a loop that repeats without end, and the end bar while one plays.
constexpr int kEndlessBars = 0x20000001;

} // namespace

PlayMap::PlayMap(int nTicksPerBar, int nEndBar)
    : mTicksPerBar(nTicksPerBar), mLoops(), mEndBar(nEndBar), mLooping(0) {
    mLoops.push_back(Loop{0, kEndlessBars, 0});
}

void PlayMap::AddLoop(int nStartBar, int nNumBars, int nPlayBar) {
    mLooping = 1;
    const Loop &last = mLoops.back();
    const int nPartBars = (nPlayBar - last.GetPlayBar()) % last.GetNumBars();
    if (nPartBars != 0) {
        mLoops.push_back(Loop{last.GetStartBar(), nPartBars, nPlayBar - nPartBars});
    }
    mLoops.push_back(Loop{nStartBar, nNumBars, nPlayBar});
}

int PlayMap::MapBar(int nBar) const {
    const Loop key{0, 0, nBar};
    auto it = std::upper_bound(mLoops.begin(), mLoops.end(), key);
    if (it != mLoops.begin()) {
        --it;
    }
    return it->GetStartBar() + (nBar - it->GetPlayBar()) % it->GetNumBars();
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
    if (it->GetNumBars() == kEndlessBars) {
        *pChangeTick = kEndlessBars;
        *pIsLast = 1;
        return;
    }

    int nChangeBar = nBar - (nBar - it->GetPlayBar()) % it->GetNumBars() + it->GetNumBars();
    *pEnd = (it->GetStartBar() + it->GetNumBars()) * mTicksPerBar;
    const auto next = it + 1;
    *pIsLast = next == mLoops.end();
    auto nextLoop = it;
    if (next != mLoops.end() && !(nChangeBar < next->GetPlayBar())) {
        nextLoop = next;
        nChangeBar = next->GetPlayBar();
    }
    *pNextStart = nextLoop->GetStartBar() * mTicksPerBar;
    *pNextLength = nextLoop->GetNumBars() * mTicksPerBar;
    *pChangeTick = nChangeBar * mTicksPerBar;
}

int PlayMap::GetEndBar() const {
    return mLooping ? kEndlessBars : mEndBar;
}

void PlayMap::ClearLoops() {
    mLoops.clear();
    mLoops.push_back(Loop{0, kEndlessBars, 0});
}

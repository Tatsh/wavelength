#include "game/rateaverager.h"

namespace {

constexpr int kNoTick = -1;

} // namespace

RateAverager::RateAverager(int nTicksPerBar) : mTicksPerBar(nTicksPerBar) {
    Reset();
}

void RateAverager::Reset() {
    mSum = 0.0f;
    mLastTick = kNoTick;
    mCount = 0;
}

void RateAverager::Sample([[maybe_unused]] int nSlot, int nTick) {
    if (mCount > 0) {
        mSum += static_cast<float>(mTicksPerBar) / static_cast<float>(nTick - mLastTick);
    }
    mLastTick = nTick;
    ++mCount;
}

float RateAverager::GetMean() const {
    return mSum / static_cast<float>(mCount);
}

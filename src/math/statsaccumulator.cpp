#include "math/statsaccumulator.h"

#include <algorithm>

StatsAccumulator::StatsAccumulator() : mMin(0.0F), mMax(0.0F), mSum(0.0F), mSorted(false) {
}

void StatsAccumulator::AddSample(float fSample) {
    mSum += fSample;
    if (mSamples.empty()) {
        mMax = fSample;
        mMin = fSample;
    } else {
        mMax = std::max(mMax, fSample);
        mMin = std::min(mMin, fSample);
    }
    mSamples.push_back(fSample);
    mSorted = false;
}

float StatsAccumulator::GetMedian() {
    if (!mSorted) {
        std::sort(mSamples.begin(), mSamples.end());
    }
    const int nCount = static_cast<int>(mSamples.size());
    const float *pSamples = mSamples.data();
    if ((nCount & 1) != 0) {
        return pSamples[nCount / 2];
    }
    // Yes, the binary averages the sample at half the count with the one after it.
    return (pSamples[nCount / 2] + pSamples[(nCount / 2) + 1]) * 0.5F;
}

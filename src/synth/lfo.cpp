#include "synth/lfo.h"

#include <math.h>

#include "os/cycles.h"

namespace {

constexpr float kPi = 3.1415927f;
constexpr float kMsPerSecond = 1000.0f;

// The triangle rises to its peak at a quarter cycle and falls to its trough at three quarters.
constexpr float kTrianglePeak = 0.25f;
constexpr float kTriangleTrough = 0.75f;
constexpr float kTriangleSlope = 4.0f;

constexpr double kSquareHalf = 0.5;

} // namespace

Lfo::Lfo() : mWaveform(kWaveformSine), mPeriod(1.0f), mPhase(0.0f) {
}

void Lfo::Start() {
    mTimer.Start();
}

void Lfo::Stop() {
    mTimer.Stop();
    mTimer.Reset();
}

float Lfo::Tick() {
    mTimer.Stop();
    mElapsedMs += static_cast<float>(mTimer.GetCycles()) * gSystemCycles2Ms;
    mTimer.Reset();
    mTimer.Start();
    if (mPeriod * kMsPerSecond <= mElapsedMs) {
        mElapsedMs = fmodf(mElapsedMs, mPeriod * kMsPerSecond);
    }
    return mElapsedMs;
}

float Lfo::Evaluate() {
    return ValueAt(Tick() / kMsPerSecond);
}

float Lfo::GetCycles(float fSeconds) const {
    return fSeconds / mPeriod;
}

float Lfo::ValueAt(float fSeconds) const {
    const float fCycle = GetCycles(fSeconds) + mPhase;
    float fValue = 0.0f;
    switch (mWaveform) {
    case kWaveformSine:
        fValue = sinf(fCycle * 2.0f * kPi);
        break;
    case kWaveformTriangle:
        if (fCycle <= kTrianglePeak) {
            fValue = fCycle * kTriangleSlope;
        } else if (fCycle <= kTriangleTrough) {
            fValue = -((fCycle - kTrianglePeak) * kTriangleSlope - 1.0f);
        } else {
            fValue = (fCycle - kTriangleTrough) * kTriangleSlope - 1.0f;
        }
        break;
    case kWaveformSquare:
        fValue = (static_cast<double>(fCycle) > kSquareHalf) ? -1.0f : 1.0f;
        break;
    case kWaveformConstant:
        fValue = 1.0f;
        break;
    default:
        break;
    }
    return fValue;
}

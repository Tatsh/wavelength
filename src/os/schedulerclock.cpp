#include "os/schedulerclock.h"

#include "os/cycles.h"
#include "os/system.h"

namespace {

constexpr float kNormalRate = 1.0f;

} // namespace

SchedulerClock::SchedulerClock(float fMs)
    : mStart(0), mElapsed(static_cast<long long>(fMs / gSystemCycles2Ms)) {
    mRate = kNormalRate;
    mRunning = 0;
}

void SchedulerClock::Start() {
    if (mRunning == 0) {
        mRunning = 1;
        mStart = SystemCycles();
    }
}

void SchedulerClock::Stop() {
    if (mRunning != 0) {
        mRunning = 0;
        const long long nCycles = static_cast<long long>(SystemCycles() - mStart);
        mElapsed += static_cast<long long>(static_cast<float>(nCycles) * mRate);
    }
}

int SchedulerClock::IsRunning() const {
    return mRunning;
}

void SchedulerClock::SetTime(float fMs) {
    mElapsed = static_cast<long long>(fMs / gSystemCycles2Ms);
    mStart = SystemCycles();
}

void SchedulerClock::SetRate(float fRate) {
    const int nWasRunning = mRunning;
    Stop();
    mRate = fRate;
    if (nWasRunning != 0) {
        Start();
    }
}

float SchedulerClock::GetRate() const {
    return mRate;
}

float SchedulerClock::GetTime() {
    if (mRunning == 0) {
        return static_cast<float>(mElapsed) * gSystemCycles2Ms;
    }
    const long long nCycles = static_cast<long long>(SystemCycles() - mStart);
    return (static_cast<float>(mElapsed) + static_cast<float>(nCycles) * mRate) * gSystemCycles2Ms;
}

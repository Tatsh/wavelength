#include "os/ramp.h"

#include "os/memfuncommand.h"

namespace {

constexpr int kNoTick = -1;
constexpr float kZero = 0.0f;

} // namespace

Ramp::Ramp(Scheduler *pScheduler, float fValue)
    // Retail stores mScheduler, then mValue, before it builds mStepCommand.
    : mStepCommand((mScheduler = pScheduler, mValue = fValue, NewMemFunCommand(this, &Ramp::Step))),
      mMoving(0), mInterp(kZero, kZero, kZero, kZero) {
    mStepTicks = kNoTick;
    mEndTick = kNoTick;
}

Ramp::~Ramp() {
    Stop();
}

void Ramp::MoveTo(int nTicks, int nStepTicks, float fTarget) {
    Move(nTicks, nStepTicks, mValue, fTarget);
}

void Ramp::Move(int nTicks, int nStepTicks, float fFrom, float fTarget) {
    if (mMoving != 0) {
        Stop();
    }
    const int nNow = mScheduler->mTick;
    if (nTicks == 0 || fTarget == fFrom) {
        Apply(fTarget, nNow);
        mValue = fTarget;
        return;
    }
    const int nEnd = nNow + nTicks;
    mStepTicks = nStepTicks;
    mValue = fFrom;
    mEndTick = nEnd;
    mInterp.Reset(fFrom, fTarget, static_cast<float>(nNow), static_cast<float>(nEnd));
    Step();
    mMoving = 1;
}

void Ramp::Stop() {
    mScheduler->Cancel(mStepCommand.Get());
    mMoving = 0;
}

float Ramp::GetValue() const {
    return mValue;
}

void Ramp::Step() {
    const int nNow = mScheduler->mTick;
    mValue = mInterp.Interp(static_cast<float>(nNow));
    Apply(mValue, nNow);
    if (nNow == mEndTick) {
        mMoving = 0;
        return;
    }
    int nNext = nNow + mStepTicks;
    if (mEndTick < nNext) {
        nNext = mEndTick;
    }
    mScheduler->PostAt(mStepCommand.Get(), nNext, false);
}

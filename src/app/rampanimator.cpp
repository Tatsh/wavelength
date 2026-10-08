#include "app/rampanimator.h"

#include <cmath>

namespace {

// The end points of the straight line a new animator follows when given no curve.
constexpr float kDefaultFromValue = 0.0f;
constexpr float kDefaultToValue = 0.0f;
constexpr float kDefaultStartTime = 0.0f;
constexpr float kDefaultEndTime = 1.0f;

// The duration of a move when the animator has no speed.
constexpr float kInstantDuration = 1.0f;

} // namespace

RampAnimator::RampAnimator(float fValue,
                           Rnd::Animatable *pAnim,
                           Interpolator *pInterp,
                           float fSpeed)
    : mValue(fValue), mElapsed(0.0f), mInterp(pInterp) {
    if (pInterp == nullptr) {
        mInterp = new LinearInterpolator(
            kDefaultFromValue, kDefaultToValue, kDefaultStartTime, kDefaultEndTime);
    }
    SetSpeed(fSpeed);
    SetTarget(fValue);
    SetAnim(pAnim);
}

RampAnimator::~RampAnimator() {
    delete mInterp;
}

void RampAnimator::SetAnim(Rnd::Animatable *pAnim) {
    mAnim = pAnim;
    if (pAnim != nullptr) {
        pAnim->SetFrame(mValue);
    }
}

void RampAnimator::SetSpeed(float fSpeed) {
    if (fSpeed == 0.0f) {
        mTimePerUnit = 0.0f;
    } else {
        mTimePerUnit = 1.0f / fSpeed;
    }
}

void RampAnimator::SetTarget(float fTarget) {
    mElapsed = 0.0f;
    if (mTimePerUnit == 0.0f) {
        // Yes, the binary holds the value where it is when there is no speed.
        mInterp->Reset(mValue, mValue, mElapsed, kInstantDuration);
    } else {
        mInterp->Reset(mValue, fTarget, mElapsed, std::fabs(fTarget - mValue) * mTimePerUnit);
    }
}

void RampAnimator::Jump(float fValue, float fTarget) {
    mValue = fValue;
    SetTarget(fTarget);
}

bool RampAnimator::Update(float fDelta, bool bForce) {
    mElapsed += std::fabs(fDelta);
    const float fValue = mInterp->Eval(mElapsed);
    if (fValue != mValue) {
        mValue = fValue;
        if (mAnim != nullptr) {
            mAnim->SetFrame(fValue);
        }
        return true;
    }
    if (bForce) {
        mAnim->SetFrame(mValue); // Yes, the binary does not test the animation here.
    }
    return false;
}

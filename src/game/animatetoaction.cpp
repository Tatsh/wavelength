#include "game/animatetoaction.h"

#include <cmath>

#include "game/triggermgr.h"
#include "os/debug.h"

namespace {

constexpr int kMinSizeWithLoop = 6;

// The curves start as the unit range. Exec() fits them.
constexpr float kUnitStart = 0.0f;
constexpr float kUnitEnd = 1.0f;
constexpr float kInvExpPower = 2.0f;
constexpr float kATanSeverity = 10.0f;

// Bring a frame into the loop from 0 to flLength.
inline float WrapFrame(float flFrame, float flLength) {
    float flWrapped = std::fmod(flFrame, flLength);
    if (flWrapped < 0.0f) {
        flWrapped += flLength;
    }
    return flWrapped;
}

} // namespace

AnimateToAction::AnimateToAction(DataArray *pAction) {
    mAnim = FindObject<Rnd::Animatable>(pAction, pAction->Sym(1));
    mTarget = pAction->Float(2);
    mDuration = pAction->Float(3);
    switch (pAction->Int(4)) {
    case kCurveLinear:
        mCurve = new LinearInterpolator(kUnitStart, kUnitStart, kUnitStart, kUnitEnd);
        break;
    case kCurveInvExp:
        mCurve = new InvExpInterpolator(kUnitStart, kUnitStart, kUnitStart, kUnitEnd, kInvExpPower);
        break;
    case kCurveATan:
        mCurve = new ATanInterpolator(kUnitStart, kUnitStart, kUnitStart, kUnitEnd, kATanSeverity);
        break;
    default:
        DebugWarn("Interp %d not 0 - 2 (file %s, line %d)",
                  pAction->Int(4),
                  pAction->mFile,
                  pAction->mLine);
        break;
    }
    mLoops = pAction->Size() >= kMinSizeWithLoop;
    if (mLoops != 0) {
        mLoopLength = pAction->Float(5);
    }
}

AnimateToAction::~AnimateToAction() {
    delete mCurve;
}

void AnimateToAction::Exec() {
    const float flNow = TheTriggerMgr.mTime[TheTriggerMgr.mClock];
    mEndTime = flNow + mDuration;
    float flFrom = mAnim->mFrame;
    if (mLoops == 0) {
        mCurve->Reset(flFrom, mTarget, flNow, mEndTime);
    } else {
        flFrom = WrapFrame(flFrom, mLoopLength);
        const float flTo = WrapFrame(mTarget, mLoopLength);
        bool bRoundTheLoop = false;
        if (flTo < flFrom) {
            if ((flTo + mLoopLength) - flFrom < flFrom - flTo) {
                mCurve->Reset(flFrom, flTo + mLoopLength, flNow, mEndTime);
                bRoundTheLoop = true;
            }
        } else if (flFrom < flTo) {
            if ((flFrom + mLoopLength) - flTo < flTo - flFrom) {
                mCurve->Reset(flFrom, flTo - mLoopLength, flNow, mEndTime);
                bRoundTheLoop = true;
            }
        }
        if (!bRoundTheLoop) {
            mCurve->Reset(flFrom, flTo, flNow, mEndTime);
        }
    }
    TheTriggerMgr.AddRunning(this);
}

bool AnimateToAction::Poll() {
    float flNow = TheTriggerMgr.mTime[TheTriggerMgr.mClock];
    if (mEndTime < flNow) {
        flNow = mEndTime;
    }
    float flFrame = mCurve->Interp(flNow);
    if (mLoops != 0) {
        flFrame = WrapFrame(flFrame, mLoopLength);
    }
    mAnim->SetFrame(flFrame);
    return flNow == mEndTime;
}

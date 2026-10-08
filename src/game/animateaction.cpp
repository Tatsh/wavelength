#include "game/animateaction.h"

#include "game/triggermgr.h"

namespace {

constexpr int kMinSizeWithStart = 4;

} // namespace

AnimateAction::AnimateAction(DataArray *pAction) {
    mAnim = FindObject<Rnd::Animatable>(pAction, pAction->Sym(1));
    mDuration = pAction->Float(2);
    mHasStart = pAction->Size() >= kMinSizeWithStart;
    if (mHasStart != 0) {
        mStart = pAction->Float(3);
    }
}

void AnimateAction::Exec() {
    if (mHasStart == 0) {
        mStart = mAnim->mFrame;
    }
    if (mDuration == 0.0f) {
        mAnim->SetFrame(mStart);
        return;
    }
    mStartTime = TheTriggerMgr.mTime[TheTriggerMgr.mClock];
    TheTriggerMgr.AddRunning(this);
}

bool AnimateAction::Poll() {
    float flElapsed = TheTriggerMgr.mTime[TheTriggerMgr.mClock] - mStartTime;
    if (mDuration < flElapsed) {
        flElapsed = mDuration;
    }
    mAnim->SetFrame(flElapsed + mStart);
    return flElapsed == mDuration;
}

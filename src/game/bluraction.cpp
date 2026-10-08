#include "game/bluraction.h"

#include "game/triggermgr.h"
#include "rnd/ps.h"

BlurAction::BlurAction(DataArray *pAction) {
    mAmount = pAction->Float(1);
    mCount = pAction->Int(2);
    DataArray *pRect = pAction->Array(3);
    for (int i = 0; i < kRectSize; ++i) {
        mRect[i] = pRect->Float(i);
    }
    mDuration = pAction->Float(4);
}

void BlurAction::Exec() {
    ThePs.mBlurAmount = mAmount;
    ThePs.mBlurCount = mCount;
    for (int i = 0; i < kRectSize; ++i) {
        ThePs.mBlurRect[i] = mRect[i];
    }
    ThePs.mBlurActive = 1;
    if (mDuration != 0.0f) {
        mEndTime = TheTriggerMgr.mTime[TheTriggerMgr.mClock] + mDuration;
        TheTriggerMgr.AddRunning(this);
    }
}

bool BlurAction::Poll() {
    if (!(mEndTime <= TheTriggerMgr.mTime[TheTriggerMgr.mClock])) {
        return false;
    }
    ThePs.mBlurActive = 0;
    return true;
}

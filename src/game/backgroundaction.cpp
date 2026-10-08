#include "game/backgroundaction.h"

#include "game/triggermgr.h"
#include "rnd/rndrenderer.h"

BackgroundAction::BackgroundAction(DataArray *pAction) {
    mColor.r = pAction->Float(1);
    mColor.g = pAction->Float(2);
    mColor.b = pAction->Float(3);
    mDuration = pAction->Float(4);
}

void BackgroundAction::Exec() {
    mStartTime = TheTriggerMgr.mTime[TheTriggerMgr.mClock];
    mStartColor = TheRnd->mClearColor;
    TheTriggerMgr.AddRunning(this);
}

bool BackgroundAction::Poll() {
    const float flElapsed = TheTriggerMgr.mTime[TheTriggerMgr.mClock] - mStartTime;
    if (mDuration <= flElapsed) {
        TheRnd->SetClearColor(mColor);
    } else {
        const float flTo = flElapsed / mDuration;
        const float flFrom = 1.0f - flTo;
        Color color;
        color.r = (mColor.r * flTo) + (mStartColor.r * flFrom);
        color.g = (mColor.g * flTo) + (mStartColor.g * flFrom);
        color.b = (mColor.b * flTo) + (mStartColor.b * flFrom);
        color.a = (mColor.a * flTo) + (mStartColor.a * flFrom);
        TheRnd->SetClearColor(color);
    }
    return mDuration <= flElapsed;
}

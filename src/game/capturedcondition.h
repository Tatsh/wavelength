#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"

/**
 * The `captured` condition, which holds when the current player captured the track they play.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 8 bytes.
 */
class CapturedCondition : public TriggerCondition {
public:
    bool Test() override {
        const TriggerMgr &mgr = TheTriggerMgr;
        return mgr.mCapturer[mgr.mTrack[mgr.mPlayer]] == mgr.mPlayer;
    }
};

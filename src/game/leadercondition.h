#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"

/**
 * The `leader` condition, which holds when the current player leads.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 8 bytes.
 */
class LeaderCondition : public TriggerCondition {
public:
    bool Test() override {
        return TheTriggerMgr.mLeader == TheTriggerMgr.mPlayer;
    }
};

#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"

/**
 * The `dying` condition, which holds while the current player is dying.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 8 bytes.
 */
class DyingCondition : public TriggerCondition {
public:
    bool Test() override {
        return TheTriggerMgr.mDying[TheTriggerMgr.mPlayer] != 0;
    }
};

#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `player` condition, which holds when the last event was of a player.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the player.
 */
class PlayerCondition : public TriggerCondition {
public:
    explicit PlayerCondition(DataArray *pCondition) : mPlayer(pCondition->Int(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mPlayer == mPlayer;
    }

private:
    int mPlayer;
};

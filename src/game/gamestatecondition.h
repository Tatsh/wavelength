#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `game_state` condition, which holds in a state of the game.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the state the begin or end event recorded.
 */
class GameStateCondition : public TriggerCondition {
public:
    explicit GameStateCondition(DataArray *pCondition) : mGameState(pCondition->Int(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mGameState == mGameState;
    }

private:
    int mGameState;
};

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
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003766b0
     * @ghidraAddress PAL: 0x003e4de0
     */
    ~GameStateCondition() override = default;

    explicit GameStateCondition(DataArray *pCondition) : mGameState(pCondition->Int(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00376730
     * @ghidraAddress PAL: 0x003e4e60
     */
    bool Test() override {
        return TheTriggerMgr.GetGameState() == mGameState;
    }

private:
    int mGameState;
};

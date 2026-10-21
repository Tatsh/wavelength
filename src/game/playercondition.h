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
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00376098
     * @ghidraAddress PAL: 0x003e47c8
     */
    ~PlayerCondition() override = default;

    explicit PlayerCondition(DataArray *pCondition) : mPlayer(pCondition->Int(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00376118
     * @ghidraAddress PAL: 0x003e4848
     */
    bool Test() override {
        return TheTriggerMgr.GetPlayer() == mPlayer;
    }

private:
    int mPlayer;
};

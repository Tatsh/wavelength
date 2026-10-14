#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `powerup` condition, which compares a powerup with TriggerMgr::mPowerup.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the powerup. No event of the shipped build writes the value it tests.
 */
class PowerupCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00376418
     * @ghidraAddress PAL: 0x003e4b48
     */
    ~PowerupCondition() override = default;

    explicit PowerupCondition(DataArray *pCondition) : mPowerup(pCondition->Int(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00376498
     * @ghidraAddress PAL: 0x003e4bc8
     */
    bool Test() override {
        return TheTriggerMgr.mPowerup == mPowerup;
    }

private:
    int mPowerup;
};

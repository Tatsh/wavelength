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
    explicit PowerupCondition(DataArray *pCondition) : mPowerup(pCondition->Int(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mPowerup == mPowerup;
    }

private:
    int mPowerup;
};

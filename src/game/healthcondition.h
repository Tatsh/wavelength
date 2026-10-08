#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `health` condition, which holds when the health of the current player is in a range.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x10 bytes. Nodes 1
 * and 2 are the ends, both included.
 */
class HealthCondition : public TriggerCondition {
public:
    explicit HealthCondition(DataArray *pCondition)
        : mLow(pCondition->Int(1)), mHigh(pCondition->Int(2)) {
    }

    bool Test() override {
        const int nHealth = TheTriggerMgr.mHealth[TheTriggerMgr.mPlayer];
        return mLow <= nHealth && nHealth <= mHigh;
    }

private:
    int mLow;
    int mHigh;
};

#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `random` condition, which holds when the random value of the last time event is in a range.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x10 bytes. Nodes 1
 * and 2 are the low end, included, and the high end, excluded.
 */
class RandomCondition : public TriggerCondition {
public:
    explicit RandomCondition(DataArray *pCondition)
        : mLow(pCondition->Float(1)), mHigh(pCondition->Float(2)) {
    }

    bool Test() override {
        return mLow <= TheTriggerMgr.mRandom && TheTriggerMgr.mRandom < mHigh;
    }

private:
    float mLow;
    float mHigh;
};

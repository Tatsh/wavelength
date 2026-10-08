#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `distance` condition, which holds when the distance of the last time event is in a range.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x10 bytes. Nodes 1
 * and 2 are the ends, both included.
 */
class DistanceCondition : public TriggerCondition {
public:
    explicit DistanceCondition(DataArray *pCondition)
        : mLow(pCondition->Float(1)), mHigh(pCondition->Float(2)) {
    }

    bool Test() override {
        return mLow <= TheTriggerMgr.mDistance && TheTriggerMgr.mDistance <= mHigh;
    }

private:
    float mLow;
    float mHigh;
};

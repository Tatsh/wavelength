#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `time` condition, which holds while the current clock is in a range.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x10 bytes. Nodes 1
 * and 2 are the ends, both included, as integers.
 */
class TimeCondition : public TriggerCondition {
public:
    explicit TimeCondition(DataArray *pCondition)
        : mLow(pCondition->Int(1)), mHigh(pCondition->Int(2)) {
    }

    bool Test() override {
        const float flTime = TheTriggerMgr.mTime[TheTriggerMgr.mClock];
        return static_cast<float>(mLow) <= flTime && flTime <= static_cast<float>(mHigh);
    }

private:
    int mLow;
    int mHigh;
};

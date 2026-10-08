#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `streak` condition, which holds when the phrase streak of the current player is in a range.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x10 bytes. Nodes 1
 * and 2 are the ends, both included.
 */
class StreakCondition : public TriggerCondition {
public:
    explicit StreakCondition(DataArray *pCondition)
        : mLow(pCondition->Int(1)), mHigh(pCondition->Int(2)) {
    }

    bool Test() override {
        const int nStreak = TheTriggerMgr.mStreak[TheTriggerMgr.mPlayer];
        return mLow <= nStreak && nStreak <= mHigh;
    }

private:
    int mLow;
    int mHigh;
};

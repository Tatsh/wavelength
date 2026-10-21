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
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375d58
     * @ghidraAddress PAL: 0x003e4488
     */
    ~StreakCondition() override = default;

    explicit StreakCondition(DataArray *pCondition)
        : mLow(pCondition->Int(1)), mHigh(pCondition->Int(2)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375dd8
     * @ghidraAddress PAL: 0x003e4508
     */
    bool Test() override {
        const int nStreak = TheTriggerMgr.GetStreak(TheTriggerMgr.GetPlayer());
        return mLow <= nStreak && nStreak <= mHigh;
    }

private:
    int mLow;
    int mHigh;
};

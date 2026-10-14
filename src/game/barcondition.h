#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `bar` condition, which holds when the bar of the last new bar event is in a range.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x10 bytes. Nodes 1
 * and 2 are the ends, both included.
 */
class BarCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00376130
     * @ghidraAddress PAL: 0x003e4860
     */
    ~BarCondition() override = default;

    explicit BarCondition(DataArray *pCondition)
        : mLow(pCondition->Int(1)), mHigh(pCondition->Int(2)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x003761b0
     * @ghidraAddress PAL: 0x003e48e0
     */
    bool Test() override {
        return mLow <= TheTriggerMgr.mBar && TheTriggerMgr.mBar <= mHigh;
    }

private:
    int mLow;
    int mHigh;
};

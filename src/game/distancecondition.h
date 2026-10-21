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
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375be0
     * @ghidraAddress PAL: 0x003e4310
     */
    ~DistanceCondition() override = default;

    explicit DistanceCondition(DataArray *pCondition)
        : mLow(pCondition->Float(1)), mHigh(pCondition->Float(2)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375c60
     * @ghidraAddress PAL: 0x003e4390
     */
    bool Test() override {
        return mLow <= TheTriggerMgr.GetDistance() && TheTriggerMgr.GetDistance() <= mHigh;
    }

private:
    float mLow;
    float mHigh;
};

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
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00376550
     * @ghidraAddress PAL: 0x003e4c80
     */
    ~HealthCondition() override = default;

    explicit HealthCondition(DataArray *pCondition)
        : mLow(pCondition->Int(1)), mHigh(pCondition->Int(2)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x003765d0
     * @ghidraAddress PAL: 0x003e4d00
     */
    bool Test() override {
        const int nHealth = TheTriggerMgr.GetHealth(TheTriggerMgr.GetPlayer());
        return mLow <= nHealth && nHealth <= mHigh;
    }

private:
    int mLow;
    int mHigh;
};

#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `gem_pos` condition, which holds when the last hit, miss, or gem event was in a lane.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the lane.
 */
class GemPosCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375e18
     * @ghidraAddress PAL: 0x003e4548
     */
    ~GemPosCondition() override = default;

    explicit GemPosCondition(DataArray *pCondition) : mGemPos(pCondition->Int(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375e98
     * @ghidraAddress PAL: 0x003e45c8
     */
    bool Test() override {
        return TheTriggerMgr.mGemPos == mGemPos;
    }

private:
    int mGemPos;
};

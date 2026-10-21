#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"

/**
 * The `leader` condition, which holds when the current player leads.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 8 bytes.
 */
class LeaderCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003764b0
     * @ghidraAddress PAL: 0x003e4be0
     */
    ~LeaderCondition() override = default;

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00376530
     * @ghidraAddress PAL: 0x003e4c60
     */
    bool Test() override {
        return TheTriggerMgr.GetLeader() == TheTriggerMgr.GetPlayer();
    }
};

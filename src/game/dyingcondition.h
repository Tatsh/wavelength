#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"

/**
 * The `dying` condition, which holds while the current player is dying.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 8 bytes.
 */
class DyingCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00376610
     * @ghidraAddress PAL: 0x003e4d40
     */
    ~DyingCondition() override = default;

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00376690
     * @ghidraAddress PAL: 0x003e4dc0
     */
    bool Test() override {
        return TheTriggerMgr.mDying[TheTriggerMgr.mPlayer] != 0;
    }
};

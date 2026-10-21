#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `music_mode` condition, which holds when the track of the current player is in a music
 * mode.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the mode.
 */
class MusicModeCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375eb0
     * @ghidraAddress PAL: 0x003e45e0
     */
    ~MusicModeCondition() override = default;

    explicit MusicModeCondition(DataArray *pCondition) : mMode(pCondition->Int(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375f30
     * @ghidraAddress PAL: 0x003e4660
     */
    bool Test() override {
        return TheTriggerMgr.GetMode(TheTriggerMgr.GetPlayer()) == mMode;
    }

private:
    int mMode;
};

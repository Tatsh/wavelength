#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The `old_screen` condition, which holds when the last screen event left a screen.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x1c bytes. Node 1
 * is the name.
 */
class OldScreenCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003759e8
     * @ghidraAddress PAL: 0x003e4118
     */
    ~OldScreenCondition() override = default;

    explicit OldScreenCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375a90
     * @ghidraAddress PAL: 0x003e41c0
     */
    bool Test() override {
        return TheTriggerMgr.mOldScreen == mName;
    }

private:
    String mName;
};

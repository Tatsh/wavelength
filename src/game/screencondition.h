#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The `screen` condition, which holds when the last component or screen event named a screen.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x1c bytes. Node 1
 * is the name.
 */
class ScreenCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375918
     * @ghidraAddress PAL: 0x003e4048
     */
    ~ScreenCondition() override = default;

    explicit ScreenCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x003759c0
     * @ghidraAddress PAL: 0x003e40f0
     */
    bool Test() override {
        return TheTriggerMgr.mScreen == mName;
    }

private:
    String mName;
};

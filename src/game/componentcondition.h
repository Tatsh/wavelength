#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The `component` condition, which holds when the last component event named a component.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x1c bytes. Node 1
 * is the name.
 */
class ComponentCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003755d8
     * @ghidraAddress PAL: 0x003e3d08
     */
    ~ComponentCondition() override = default;

    explicit ComponentCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375680
     * @ghidraAddress PAL: 0x003e3db0
     */
    bool Test() override {
        return TheTriggerMgr.mComponent == mName;
    }

private:
    String mName;
};

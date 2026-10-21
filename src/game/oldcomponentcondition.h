#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The `old_component` condition, which holds when the focus left a component.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x1c bytes. Node 1
 * is the name.
 */
class OldComponentCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003756a8
     * @ghidraAddress PAL: 0x003e3dd8
     */
    ~OldComponentCondition() override = default;

    explicit OldComponentCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375750
     * @ghidraAddress PAL: 0x003e3e80
     */
    bool Test() override {
        return TheTriggerMgr.GetOldComponent() == mName;
    }

private:
    String mName;
};

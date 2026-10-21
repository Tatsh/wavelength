#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The `panel` condition, which holds when the last component event named a panel.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x1c bytes. Node 1
 * is the name.
 */
class PanelCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375778
     * @ghidraAddress PAL: 0x003e3ea8
     */
    ~PanelCondition() override = default;

    explicit PanelCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375820
     * @ghidraAddress PAL: 0x003e3f50
     */
    bool Test() override {
        return TheTriggerMgr.GetPanel() == mName;
    }

private:
    String mName;
};

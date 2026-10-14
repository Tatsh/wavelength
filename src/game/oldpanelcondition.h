#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The `old_panel` condition, which compares a name with TriggerMgr::mOldPanel.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x1c bytes. Node 1
 * is the name. No event of the shipped build writes the value it tests.
 */
class OldPanelCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375848
     * @ghidraAddress PAL: 0x003e3f78
     */
    ~OldPanelCondition() override = default;

    explicit OldPanelCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x003758f0
     * @ghidraAddress PAL: 0x003e4020
     */
    bool Test() override {
        return TheTriggerMgr.mOldPanel == mName;
    }

private:
    String mName;
};

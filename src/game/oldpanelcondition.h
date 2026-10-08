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
    explicit OldPanelCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mOldPanel == mName;
    }

private:
    String mName;
};

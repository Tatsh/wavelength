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
    explicit OldScreenCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mOldScreen == mName;
    }

private:
    String mName;
};

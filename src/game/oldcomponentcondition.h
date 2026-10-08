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
    explicit OldComponentCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mOldComponent == mName;
    }

private:
    String mName;
};

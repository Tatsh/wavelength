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
    explicit ComponentCondition(DataArray *pCondition) : mName(pCondition->Sym(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mComponent == mName;
    }

private:
    String mName;
};

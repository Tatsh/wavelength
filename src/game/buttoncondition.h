#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `button` condition, which holds when the last button event included a button.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the number of the button bit.
 */
class ButtonCondition : public TriggerCondition {
public:
    explicit ButtonCondition(DataArray *pCondition) : mMask(1 << pCondition->Int(1)) {
    }

    bool Test() override {
        return (TheTriggerMgr.mButtons & mMask) != 0;
    }

private:
    int mMask;
};

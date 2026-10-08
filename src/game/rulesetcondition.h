#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `rule_set` condition, which holds under the rule set of the last begin event.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the rule set.
 */
class RuleSetCondition : public TriggerCondition {
public:
    explicit RuleSetCondition(DataArray *pCondition) : mRuleSet(pCondition->Int(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mRuleSet == mRuleSet;
    }

private:
    int mRuleSet;
};

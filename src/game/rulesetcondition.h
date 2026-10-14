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
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00376748
     * @ghidraAddress PAL: 0x003e4e78
     */
    ~RuleSetCondition() override = default;

    explicit RuleSetCondition(DataArray *pCondition) : mRuleSet(pCondition->Int(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x003767c8
     * @ghidraAddress PAL: 0x003e4ef8
     */
    bool Test() override {
        return TheTriggerMgr.mRuleSet == mRuleSet;
    }

private:
    int mRuleSet;
};

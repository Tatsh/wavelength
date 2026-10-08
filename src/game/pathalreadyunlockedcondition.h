#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `path_already_unlocked` condition, which holds when a path of the arena is unlocked.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the path.
 */
class PathAlreadyUnlockedCondition : public TriggerCondition {
public:
    explicit PathAlreadyUnlockedCondition(DataArray *pCondition) : mPath(pCondition->Int(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.IsPathUnlocked(mPath);
    }

private:
    int mPath;
};

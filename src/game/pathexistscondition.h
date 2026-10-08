#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `path_exists` condition, which holds when the arena has a path.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the path.
 */
class PathExistsCondition : public TriggerCondition {
public:
    explicit PathExistsCondition(DataArray *pCondition) : mPath(pCondition->Int(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.PathExists(mPath);
    }

private:
    int mPath;
};

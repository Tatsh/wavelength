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
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00376888
     * @ghidraAddress PAL: 0x003e4fb8
     */
    ~PathExistsCondition() override = default;

    explicit PathExistsCondition(DataArray *pCondition) : mPath(pCondition->Int(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00376908
     * @ghidraAddress PAL: 0x003e5038
     */
    bool Test() override {
        return TheTriggerMgr.PathExists(mPath);
    }

private:
    int mPath;
};

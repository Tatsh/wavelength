#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `score` condition, which holds when the score of the current player is in a range.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x10 bytes. Nodes 1
 * and 2 are the ends, both included.
 */
class ScoreCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003762b0
     * @ghidraAddress PAL: 0x003e49e0
     */
    ~ScoreCondition() override = default;

    explicit ScoreCondition(DataArray *pCondition)
        : mLow(pCondition->Int(1)), mHigh(pCondition->Int(2)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00376330
     * @ghidraAddress PAL: 0x003e4a60
     */
    bool Test() override {
        const int nScore = TheTriggerMgr.mScore[TheTriggerMgr.mPlayer];
        return mLow <= nScore && nScore <= mHigh;
    }

private:
    int mLow;
    int mHigh;
};

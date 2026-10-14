#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `track` condition, which holds when the current player plays a track.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the track.
 */
class TrackCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00376370
     * @ghidraAddress PAL: 0x003e4aa0
     */
    ~TrackCondition() override = default;

    explicit TrackCondition(DataArray *pCondition) : mTrack(pCondition->Int(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x003763f0
     * @ghidraAddress PAL: 0x003e4b20
     */
    bool Test() override {
        return TheTriggerMgr.mTrack[TheTriggerMgr.mPlayer] == mTrack;
    }

private:
    int mTrack;
};

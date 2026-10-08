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
    explicit TrackCondition(DataArray *pCondition) : mTrack(pCondition->Int(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mTrack[TheTriggerMgr.mPlayer] == mTrack;
    }

private:
    int mTrack;
};

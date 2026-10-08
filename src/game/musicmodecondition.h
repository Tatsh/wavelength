#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `music_mode` condition, which holds when the track of the current player is in a music
 * mode.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the mode.
 */
class MusicModeCondition : public TriggerCondition {
public:
    explicit MusicModeCondition(DataArray *pCondition) : mMode(pCondition->Int(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mMode[TheTriggerMgr.mPlayer] == mMode;
    }

private:
    int mMode;
};

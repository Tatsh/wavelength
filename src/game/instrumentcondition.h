#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"
#include "script/dataarray.h"

/**
 * The `instrument` condition, which holds when the track of the current player is of an
 * instrument.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0xc bytes. Node 1 is
 * the instrument.
 */
class InstrumentCondition : public TriggerCondition {
public:
    explicit InstrumentCondition(DataArray *pCondition) : mInstrument(pCondition->Int(1)) {
    }

    bool Test() override {
        return TheTriggerMgr.mInstrument[TheTriggerMgr.mPlayer] == mInstrument;
    }

private:
    int mInstrument;
};

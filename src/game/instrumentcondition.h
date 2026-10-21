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
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375f58
     * @ghidraAddress PAL: 0x003e4688
     */
    ~InstrumentCondition() override = default;

    explicit InstrumentCondition(DataArray *pCondition) : mInstrument(pCondition->Int(1)) {
    }

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x00375fd8
     * @ghidraAddress PAL: 0x003e4708
     */
    bool Test() override {
        return TheTriggerMgr.GetInstrument(TheTriggerMgr.GetPlayer()) == mInstrument;
    }

private:
    int mInstrument;
};

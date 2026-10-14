#pragma once

#include "game/triggercondition.h"
#include "game/triggermgr.h"

/**
 * The `captured` condition, which holds when the current player captured the track they play.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 8 bytes.
 */
class CapturedCondition : public TriggerCondition {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00375528
     * @ghidraAddress PAL: 0x003e3c58
     */
    ~CapturedCondition() override = default;

    /**
     * Test the condition.
     *
     * @return Whether the condition holds.
     * @ghidraAddress NTSC-U/C: 0x003755a8
     * @ghidraAddress PAL: 0x003e3cd8
     */
    bool Test() override {
        const TriggerMgr &mgr = TheTriggerMgr;
        return mgr.mCapturer[mgr.mTrack[mgr.mPlayer]] == mgr.mPlayer;
    }
};

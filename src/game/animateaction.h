#pragma once

#include "game/triggeraction.h"
#include "rnd/animatable.h"
#include "script/dataarray.h"

/**
 * The `animate` action, which plays an animatable forward in step with the clock.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x20 bytes. Node 1 names
 * the animatable and node 2 is the length to play. The optional node 3 is the first frame, and
 * without it the animatable plays on from its current frame.
 */
class AnimateAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00201e60
     * @ghidraAddress PAL: 0x0020ac00
     */
    explicit AnimateAction(DataArray *pAction);

    /**
     * Start playing, or move to the first frame at once when the length is 0.
     *
     * @ghidraAddress NTSC-U/C: 0x00202d80
     * @ghidraAddress PAL: 0x0020bb20
     */
    void Exec() override;

    /**
     * Move the animatable to the frame for the time on the clock.
     *
     * @return Whether the length has played.
     * @ghidraAddress NTSC-U/C: 0x002032c8
     * @ghidraAddress PAL: 0x0020c080
     */
    bool Poll() override;

private:
    Rnd::Animatable *mAnim;
    float mStart;
    float mDuration;
    float mStartTime;
    int mHasStart;
};

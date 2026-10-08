#pragma once

#include "game/triggercondition.h"
#include "rnd/animatable.h"
#include "script/dataarray.h"

/**
 * The `anim` condition, which holds once when an animatable passes a frame.
 *
 * The RTTI records the class as deriving from TriggerCondition. The object is 0x14 bytes. Node 1
 * names the animatable and node 2 is the frame. The condition holds when the filtered frame of the
 * animatable has reached the frame since the last test and was short of it before.
 */
class AnimCondition : public TriggerCondition {
public:
    /**
     * Read the condition.
     *
     * The frame of the last test starts indeterminate.
     *
     * @param pCondition The node.
     * @ghidraAddress NTSC-U/C: 0x00203bd8
     * @ghidraAddress PAL: 0x0020c990
     */
    explicit AnimCondition(DataArray *pCondition);

    bool Test() override {
        const float flFrame = mAnim->mFilteredFrame;
        const bool bPassed = mLastFrame < mFrame && mFrame <= flFrame;
        mLastFrame = flFrame;
        return bPassed;
    }

private:
    Rnd::Animatable *mAnim;
    float mFrame;
    float mLastFrame;
};

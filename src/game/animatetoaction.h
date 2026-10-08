#pragma once

#include "game/triggeraction.h"
#include "math/interpolator.h"
#include "rnd/animatable.h"
#include "script/dataarray.h"

/**
 * The `animate_to` action, which moves an animatable to a frame along a curve.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x28 bytes. Node 1 names
 * the animatable, node 2 is the frame to move to, node 3 the time the move takes, and node 4 the
 * curve, 0 for a line, 1 for an InvExpInterpolator, and 2 for an ATanInterpolator. The optional
 * node 5 is the length of a looping animation, and the move then takes the shorter way round.
 */
class AnimateToAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00201bd8
     * @ghidraAddress PAL: 0x0020a978
     */
    explicit AnimateToAction(DataArray *pAction);

    /**
     * Delete the curve.
     *
     * @ghidraAddress NTSC-U/C: 0x00201de8
     * @ghidraAddress PAL: 0x0020ab88
     */
    ~AnimateToAction() override;

    /**
     * Fit the curve from the current frame and start the move.
     *
     * @ghidraAddress NTSC-U/C: 0x00202ba0
     * @ghidraAddress PAL: 0x0020b940
     */
    void Exec() override;

    /**
     * Move the animatable to the frame of the curve for the time on the clock.
     *
     * @return Whether the time of the move has passed.
     * @ghidraAddress NTSC-U/C: 0x00203350
     * @ghidraAddress PAL: 0x0020c108
     */
    bool Poll() override;

private:
    // The values of node 4.
    enum Curve {
        kCurveLinear = 0,
        kCurveInvExp = 1,
        kCurveATan = 2,
    };

    Rnd::Animatable *mAnim;
    float mTarget;
    float mDuration;
    Interpolator *mCurve;
    int mLoops;
    float mLoopLength;
    float mEndTime;
};

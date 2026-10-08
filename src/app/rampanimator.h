#pragma once

#include "math/interpolator.h"
#include "rnd/animatable.h"

/**
 * Value that moves toward a target along an interpolator, and poses an animation on the value.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x14 bytes. The title is inferred
 * from the role. The head-up display panels embed one as their first member to slide in and out.
 */
class RampAnimator {
public:
    /**
     * Construct an animator resting at a value.
     *
     * @param fValue The value, and the first target.
     * @param pAnim The animation posed on the value, or null.
     * @param pInterp The curve the value follows, or null for a straight line. The animator owns
     *                the curve.
     * @param fSpeed The change per unit of time, or 0 for a move that takes one unit of time.
     * @ghidraAddress NTSC-U/C: 0x001e41c8
     * @ghidraAddress PAL: 0x001ecf68
     */
    RampAnimator(float fValue, Rnd::Animatable *pAnim, Interpolator *pInterp, float fSpeed);

    /**
     * Destroy the animator and its curve.
     *
     * @ghidraAddress NTSC-U/C: 0x001e4270
     * @ghidraAddress PAL: 0x001ed010
     */
    ~RampAnimator();

    /**
     * Pose an animation on the value.
     *
     * @param pAnim The animation, or null.
     * @ghidraAddress NTSC-U/C: 0x001e42d0
     * @ghidraAddress PAL: 0x001ed070
     */
    void SetAnim(Rnd::Animatable *pAnim);

    /**
     * Set the speed of a move.
     *
     * @param fSpeed The change per unit of time, or 0 for a move that takes one unit of time.
     * @ghidraAddress NTSC-U/C: 0x001e4300
     * @ghidraAddress PAL: 0x001ed0a0
     */
    void SetSpeed(float fSpeed);

    /**
     * Start moving from the current value toward a target.
     *
     * @param fTarget The target.
     * @ghidraAddress NTSC-U/C: 0x001e4330
     * @ghidraAddress PAL: 0x001ed0d0
     */
    void SetTarget(float fTarget);

    /**
     * Move to a value at once and start moving from it toward a target.
     *
     * @param fValue The value.
     * @param fTarget The target.
     * @ghidraAddress NTSC-U/C: 0x001e43c0
     * @ghidraAddress PAL: 0x001ed160
     */
    void Jump(float fValue, float fTarget);

    /**
     * Advance the move and pose the animation on the new value.
     *
     * @param fDelta The time since the last update. Its magnitude is used.
     * @param bForce Whether to pose the animation when the value did not change.
     * @return Whether the value changed.
     * @ghidraAddress NTSC-U/C: 0x001e43e0
     * @ghidraAddress PAL: 0x001ed180
     */
    bool Update(float fDelta, bool bForce);

    float mValue;           /*!< The current value. */
    float mElapsed;         /*!< The time since the move started. */
    float mTimePerUnit;     /*!< The time per unit of change, or 0. */
    Interpolator *mInterp;  /*!< The curve the value follows. */
    Rnd::Animatable *mAnim; /*!< The animation posed on the value, or null. */
};

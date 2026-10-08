#pragma once

#include "math/interpolator.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/scheduler.h"

/**
 * Value that moves linearly to a target over song ticks, applied in steps by a scheduled command.
 *
 * The RTTI includes the class name. A derived class supplies Apply(), which receives each new
 * value. Only the members its callers here use are declared.
 */
class Ramp {
public:
    /**
     * Construct a ramp that holds a value.
     *
     * @param pScheduler The scheduler the steps are queued on.
     * @param fValue The starting value.
     * @ghidraAddress NTSC-U/C: 0x002817f8
     * @ghidraAddress PAL: 0x0028b0f8
     */
    Ramp(Scheduler *pScheduler, float fValue);

    /**
     * Stop the ramp.
     *
     * @ghidraAddress NTSC-U/C: 0x00281880
     * @ghidraAddress PAL: 0x0028b180
     */
    virtual ~Ramp();

    /**
     * Apply a new value.
     *
     * @param fValue The value.
     * @param nTick The song tick the value applies at.
     */
    virtual void Apply(float fValue, int nTick) = 0;

    /**
     * Move from the current value to a target.
     *
     * @param nTicks The ticks the move lasts. Zero applies the target at once.
     * @param nStepTicks The ticks between steps.
     * @param fTarget The target.
     * @ghidraAddress NTSC-U/C: 0x002818e8
     * @ghidraAddress PAL: 0x0028b1e8
     */
    void MoveTo(int nTicks, int nStepTicks, float fTarget);

    /**
     * Move from one value to another.
     *
     * A move of no ticks, or between equal values, applies the target at once.
     *
     * @param nTicks The ticks the move lasts.
     * @param nStepTicks The ticks between steps.
     * @param fFrom The starting value.
     * @param fTarget The target.
     * @ghidraAddress NTSC-U/C: 0x00281910
     * @ghidraAddress PAL: 0x0028b210
     */
    void Move(int nTicks, int nStepTicks, float fFrom, float fTarget);

    /**
     * Withdraw the next step, leaving the value where it is.
     *
     * @ghidraAddress NTSC-U/C: 0x002819f0
     * @ghidraAddress PAL: 0x0028b2f0
     */
    void Stop();

    /**
     * Report the current value.
     *
     * @return The value.
     * @ghidraAddress NTSC-U/C: 0x00281a28
     * @ghidraAddress PAL: 0x0028b328
     */
    float GetValue() const;

private:
    /**
     * Apply the value of the current tick, and queue the next step until the move ends.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00281a30
     * @ghidraAddress PAL: 0x0028b330
     */
    void Step();

    float mValue;               // The current value.
    Scheduler *mScheduler;      // The scheduler the steps are queued on.
    Ptr<Command> mStepCommand;  // The command that runs Step().
    int mMoving;                // Non-zero while a step is queued.
    LinearInterpolator mInterp; // The value at each tick of the move.
    int mEndTick;               // The tick the move ends at, or -1.
    int mStepTicks;             // The ticks between steps, or -1.
};

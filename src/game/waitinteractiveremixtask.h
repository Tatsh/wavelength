#pragma once

#include "game/gamecallback.h"
#include "game/remixtutorial.h"
#include "os/task.h"
#include "script/dataarray.h"

/**
 * Task that waits until the player performs the remix actions a tutorial step lists.
 *
 * The RTTI includes the class name and records Task and GameCallback as the bases. A step has the
 * shape WaitInteractiveTask reads, with remix conditions. Only `pitch` resets its count when it
 * fires.
 */
class WaitInteractiveRemixTask : public Task, public GameCallback {
public:
    /**
     * Construct a task for one tutorial step.
     *
     * @param pStep The step, with the conditions to wait for.
     * @param pTutorial The tutorial the step belongs to.
     * @ghidraAddress NTSC-U/C: 0x00142290
     * @ghidraAddress PAL: 0x00143c30
     */
    WaitInteractiveRemixTask(DataArray *pStep, RemixTutorial *pTutorial);

    /**
     * Count a pitch change, restart the idle times, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x00142728
     * @ghidraAddress PAL: 0x001440c8
     */
    void OnPitch() override;

    /**
     * Record that looping was switched on, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x00142768
     * @ghidraAddress PAL: 0x00144108
     */
    void OnLoop() override;

    /**
     * Record a section change, restart the idle times, and evaluate the conditions.
     *
     * @param nSection The section, which the task ignores.
     * @ghidraAddress NTSC-U/C: 0x00142788
     * @ghidraAddress PAL: 0x00144128
     */
    void OnSectionChange(int nSection) override;

    /**
     * Record a burned pattern, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x001427c8
     * @ghidraAddress PAL: 0x00144168
     */
    void OnBurnPattern() override;

    /**
     * Count an erased pattern, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x001427e8
     * @ghidraAddress PAL: 0x00144188
     */
    void OnErase() override;

    /**
     * Record an erased section, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x00142840
     * @ghidraAddress PAL: 0x001441e0
     */
    void OnEraseSection() override;

    /**
     * Count a rotation, and evaluate the conditions.
     *
     * @param bRight The rotation was to the right, which adds one. Otherwise it subtracts one.
     * @ghidraAddress NTSC-U/C: 0x00142810
     * @ghidraAddress PAL: 0x001441b0
     */
    void OnRotate(bool bRight) override;

    /**
     * Record that the pattern menu was opened, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x00142860
     * @ghidraAddress PAL: 0x00144200
     */
    void OnPatternMenuOpen() override;

    RemixTutorial *mTutorial; /*!< The tutorial the step belongs to. */
    DataArray *mStep;         /*!< The step. */
    int mPitchCount;          /*!< Pitch changes since `pitch` last fired. */
    float mIdleTime;          /*!< Song clock time `idle` counts from. */
    float mIdle2Time;         /*!< Song clock time `idle2` counts from. */
    int mLoopOn;              /*!< Nonzero once looping was switched on. */
    int mForwardSection;      /*!< Nonzero once the section changed. */
    int mBurnPattern;         /*!< Nonzero once a pattern was burned. */
    int mEraseCount;          /*!< Erased patterns. */
    int mRotation;            /*!< Rotations to the right less rotations to the left. */
    int mEraseSection;        /*!< Nonzero once a section was erased. */
    int mPatternMenuOpen;     /*!< Nonzero once the pattern menu was opened. */

protected:
    /**
     * Install the task as TheGameCallback, and restart the idle times.
     *
     * @ghidraAddress NTSC-U/C: 0x001422e8
     * @ghidraAddress PAL: 0x00143c88
     */
    void OnStart() override;

    /**
     * Stop reporting to the task.
     *
     * @ghidraAddress NTSC-U/C: 0x00142328
     * @ghidraAddress PAL: 0x00143cc8
     */
    void OnStop() override;

    /**
     * Run the actions of the idle conditions whose time has passed.
     *
     * @ghidraAddress NTSC-U/C: 0x00142348
     * @ghidraAddress PAL: 0x00143ce8
     */
    void OnPoll() override;

private:
    /**
     * Evaluate every condition of the step.
     *
     * @ghidraAddress NTSC-U/C: 0x00142440
     * @ghidraAddress PAL: 0x00143de0
     */
    void EvaluateConditions();

    /**
     * Run the actions of a condition when it holds.
     *
     * @param pCondition The condition.
     * @ghidraAddress NTSC-U/C: 0x001424b8
     * @ghidraAddress PAL: 0x00143e58
     */
    void EvaluateCondition(DataArray *pCondition);

    /**
     * Run the actions of a condition.
     *
     * @param pCondition The condition.
     * @param nFirst The index of the first action.
     * @ghidraAddress NTSC-U/C: 0x00142660
     * @ghidraAddress PAL: 0x00144000
     */
    void RunActions(DataArray *pCondition, int nFirst);
};

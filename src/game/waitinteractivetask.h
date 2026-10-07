#pragma once

#include "game/gamecallback.h"
#include "game/tutorialgamelogic.h"
#include "os/task.h"
#include "script/dataarray.h"

/**
 * Task that waits until the player performs the game actions a tutorial step lists.
 *
 * The RTTI includes the class name and records Task and GameCallback as the bases. Each node of
 * the step after the first is a condition, an array whose first node is the condition name and
 * whose later nodes are the actions to run when the condition holds. An action is a script command,
 * or the symbol `done`, which finishes the task. The counts reset when their condition fires.
 */
class WaitInteractiveTask : public Task, public GameCallback {
public:
    /**
     * Construct a task for one tutorial step.
     *
     * @param pStep The step, with the conditions to wait for.
     * @param pLogic The tutorial the step belongs to.
     * @ghidraAddress NTSC-U/C: 0x001419f0
     * @ghidraAddress PAL: 0x00143390
     */
    WaitInteractiveTask(DataArray *pStep, TutorialGameLogic *pLogic);

    /**
     * Count a hit gem, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x00141cd8
     * @ghidraAddress PAL: 0x00143678
     */
    void OnGemHit() override;

    /**
     * Count a missed gem, and evaluate the conditions.
     *
     * @param nLane The lane of the gem.
     * @ghidraAddress NTSC-U/C: 0x00141c98
     * @ghidraAddress PAL: 0x00143638
     */
    void OnGemMiss(int nLane) override;

    /**
     * Count a gem that passed, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x00141c68
     * @ghidraAddress PAL: 0x00143608
     */
    void OnGemPass() override;

    /**
     * Count a capture, and evaluate the conditions.
     *
     * @param nTrack The captured track.
     * @ghidraAddress NTSC-U/C: 0x00141d10
     * @ghidraAddress PAL: 0x001436b0
     */
    void OnCapture(int nTrack) override;

    /**
     * Count an automatic capture, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x00141d58
     * @ghidraAddress PAL: 0x001436f8
     */
    void OnAutocapture() override;

    /**
     * Count a broken streak, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x00141d88
     * @ghidraAddress PAL: 0x00143728
     */
    void OnStreakBroken() override;

    /**
     * Count a deployed powerup, and evaluate the conditions.
     *
     * @ghidraAddress NTSC-U/C: 0x00141dc0
     * @ghidraAddress PAL: 0x00143760
     */
    void OnPowerup() override;

    TutorialGameLogic *mLogic; /*!< The tutorial the step belongs to. */
    DataArray *mStep;          /*!< The step. */
    int mPassCount;            /*!< Gems passed since the last hit, miss, or capture. */
    int mMissCount;            /*!< Gems missed since the last hit, pass, or capture. */
    int mHitCount;             /*!< Gems hit since the last miss or pass. */
    int mCaptureCount;         /*!< Captures. */
    int mNthCapture;           /*!< Captures, set past every count once `nth_capture` fires. */
    int mCaptureFailCount;     /*!< Broken streaks. */
    int mMissLane;             /*!< The lane OnGemMiss() last reported. */
    float mPromptTime;         /*!< Song clock time `delay` and `inactive_delay` count from. */
    float mActiveIdleTime;     /*!< Song clock time `active_idle` counts from. */
    int mPowerupCount;         /*!< Deployed powerups. */
    int mStreak; /*!< One more than the last captured track, negated once the streak breaks. */
    int mAutocaptureCount; /*!< Automatic captures. */
    int mMissStreak;       /*!< Gems missed since the last hit or pass. */
    int mMashCount;        /*!< Times `button_mash` held without firing. */

protected:
    /**
     * Install the task as TheGameCallback, and reset the counts and the times.
     *
     * @ghidraAddress NTSC-U/C: 0x00141a58
     * @ghidraAddress PAL: 0x001433f8
     */
    void OnStart() override;

    /**
     * Stop reporting to the task.
     *
     * @ghidraAddress NTSC-U/C: 0x00141ac8
     * @ghidraAddress PAL: 0x00143468
     */
    void OnStop() override;

    /**
     * Run the actions of the timed conditions whose time has passed.
     *
     * @ghidraAddress NTSC-U/C: 0x00141ae8
     * @ghidraAddress PAL: 0x00143488
     */
    void OnPoll() override;

private:
    /**
     * Evaluate every condition of the step.
     *
     * @ghidraAddress NTSC-U/C: 0x00141de8
     * @ghidraAddress PAL: 0x00143788
     */
    void EvaluateConditions();

    /**
     * Run the actions of a condition when it holds.
     *
     * @param pCondition The condition.
     * @ghidraAddress NTSC-U/C: 0x00141e60
     * @ghidraAddress PAL: 0x00143800
     */
    void EvaluateCondition(DataArray *pCondition);

    /**
     * Run the actions of a condition.
     *
     * @param pCondition The condition.
     * @param nFirst The index of the first action.
     * @ghidraAddress NTSC-U/C: 0x001421c8
     * @ghidraAddress PAL: 0x00143b68
     */
    void RunActions(DataArray *pCondition, int nFirst);
};

#pragma once

#include "game/remixlogic.h"
#include "game/streamqueue.h"
#include "os/serialtasks.h"
#include "os/task.h"
#include "os/taskdonenotifier.h"
#include "script/dataarray.h"

/**
 * Scripted lesson that runs on top of a remix session.
 *
 * The RTTI includes the class name and records TaskDoneNotifier as the base. The "script" entry of
 * the song's "Tutorial" configuration lists the steps, each a task that mScript runs in order. The
 * script commands the constructor registers switch the remix features on and off, play narration
 * streams, and drive the remix controls. While the tutorial exists, the first player's controller
 * bindings are the defaults.
 */
class RemixTutorial : public TaskDoneNotifier {
public:
    /**
     * Register the tutorial's script commands, build the steps of the song's script, and switch
     * the first player to the default controller bindings.
     *
     * @param pLogic The remix session the tutorial drives.
     * @ghidraAddress NTSC-U/C: 0x00136928
     * @ghidraAddress PAL: 0x00138140
     */
    explicit RemixTutorial(RemixLogic *pLogic);

    /**
     * Remove the script commands, stop the steps and the narration, and restore the first
     * player's controller bindings.
     *
     * @ghidraAddress NTSC-U/C: 0x00136d10
     * @ghidraAddress PAL: 0x00138570
     */
    ~RemixTutorial() override;

    /**
     * Ignore the request. A tutorial never waits for named work.
     *
     * @param pTask The task waiting for the work.
     * @param pszName The name of the work.
     * @ghidraAddress NTSC-U/C: 0x0033fcf8
     * @ghidraAddress PAL: 0x003ad230
     */
    void NotifyWhenDone([[maybe_unused]] Task *pTask,
                        [[maybe_unused]] const char *pszName) override {
    }

    /**
     * Start the first step.
     *
     * @ghidraAddress NTSC-U/C: 0x00136ea8
     * @ghidraAddress PAL: 0x00138708
     */
    void Start();

    /**
     * Advance the steps and the narration, unless the tutorial is paused.
     *
     * @return False once the last step is done, and true otherwise.
     * @ghidraAddress NTSC-U/C: 0x00136ec8
     * @ghidraAddress PAL: 0x00138728
     */
    bool Poll();

    /**
     * Pause or resume the steps and the narration.
     *
     * @param bPaused Pause the tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136f38
     * @ghidraAddress PAL: 0x00138798
     */
    void SetPaused(bool bPaused);

    /**
     * Report a pitch change to TheGameCallback.
     *
     * @ghidraAddress NTSC-U/C: 0x00136f60
     * @ghidraAddress PAL: 0x001387c0
     */
    void NotifyPitch();

    /**
     * Report to TheGameCallback that looping was switched on.
     *
     * @ghidraAddress NTSC-U/C: 0x00136f98
     * @ghidraAddress PAL: 0x001387f8
     */
    void NotifyLoop();

    /**
     * Report a section change to TheGameCallback.
     *
     * @param nSection The section.
     * @ghidraAddress NTSC-U/C: 0x00136fd0
     * @ghidraAddress PAL: 0x00138830
     */
    void NotifySectionChange(int nSection);

    /**
     * Report a rotation to TheGameCallback when it differs from the last one, and record it.
     *
     * @param bRight The rotation was to the right.
     * @param nRotation The rotation.
     * @ghidraAddress NTSC-U/C: 0x00137018
     * @ghidraAddress PAL: 0x00138878
     */
    void NotifyRotate(bool bRight, int nRotation);

    /**
     * Queue a narration stream.
     *
     * @param pszName The stream name.
     * @param bSkipIfBusy Drop the request when a stream is already queued.
     * @ghidraAddress NTSC-U/C: 0x00137190
     * @ghidraAddress PAL: 0x001389f0
     */
    void QueueStream(const char *pszName, bool bSkipIfBusy);

    /**
     * Clear the gems of every section.
     *
     * @ghidraAddress NTSC-U/C: 0x001371b0
     * @ghidraAddress PAL: 0x00138a10
     */
    void ClearGems();

    /**
     * Open the remix menu.
     *
     * @param bEnable Ignored.
     * @ghidraAddress NTSC-U/C: 0x001371d0
     * @ghidraAddress PAL: 0x00138a30
     */
    void EnableMenu(bool bEnable);

    /**
     * Open or close the tempo control.
     *
     * @param bOpen Open the control.
     * @ghidraAddress NTSC-U/C: 0x001371f0
     * @ghidraAddress PAL: 0x00138a50
     */
    void OpenTempo(bool bOpen);

    /**
     * Raise or lower the tempo.
     *
     * @param nSteps The steps to raise the tempo by, negative to lower it.
     * @ghidraAddress NTSC-U/C: 0x00137220
     * @ghidraAddress PAL: 0x00138a80
     */
    void AdjustTempo(int nSteps);

    /**
     * Play the second sound of the interface sound bank.
     *
     * @ghidraAddress NTSC-U/C: 0x00137298
     * @ghidraAddress PAL: 0x00138af8
     */
    void PlaySound();

    /**
     * Ignore the request.
     *
     * @param bAllow Ignored.
     * @ghidraAddress NTSC-U/C: 0x001372b8
     * @ghidraAddress PAL: 0x00138b18
     */
    void AllowPatternCancel(bool bAllow);

    /**
     * Run the "stream" script command, which queues the narration stream it identifies.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136560
     * @ghidraAddress PAL: 0x00137d78
     */
    static void OnStream(DataArray *pCommand, void *pUserData);

    /**
     * Run the "enable_looping" script command, which turns looping of the first lane on or off.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x001365e0
     * @ghidraAddress PAL: 0x00137df8
     */
    static void OnEnableLooping(DataArray *pCommand, void *pUserData);

    /**
     * Run the "print" script command, which does nothing.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136618
     * @ghidraAddress PAL: 0x00137e30
     */
    static void OnPrint(DataArray *pCommand, void *pUserData);

    /**
     * Run the "disable_rot" script command, which disables or enables rotation.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136620
     * @ghidraAddress PAL: 0x00137e38
     */
    static void OnDisableRot(DataArray *pCommand, void *pUserData);

    /**
     * Run the "disable_pitching" script command, which disables or enables pitching.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136650
     * @ghidraAddress PAL: 0x00137e68
     */
    static void OnDisablePitching(DataArray *pCommand, void *pUserData);

    /**
     * Run the "disable_section_change" script command, which disables or enables section
     * changes.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136680
     * @ghidraAddress PAL: 0x00137e98
     */
    static void OnDisableSectionChange(DataArray *pCommand, void *pUserData);

    /**
     * Run the "disable_hud" script command, which hides or shows the display.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x001366b0
     * @ghidraAddress PAL: 0x00137ec8
     */
    static void OnDisableHud(DataArray *pCommand, void *pUserData);

    /**
     * Run the "disable_erasing" script command, which disables or enables erasing.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x001366e0
     * @ghidraAddress PAL: 0x00137ef8
     */
    static void OnDisableErasing(DataArray *pCommand, void *pUserData);

    /**
     * Run the "allow_looping" script command, which sets whether looping is allowed.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136710
     * @ghidraAddress PAL: 0x00137f28
     */
    static void OnAllowLooping(DataArray *pCommand, void *pUserData);

    /**
     * Run the "clear_gems" script command, which clears the gems.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136748
     * @ghidraAddress PAL: 0x00137f60
     */
    static void OnClearGems(DataArray *pCommand, void *pUserData);

    /**
     * Run the "show_patterns" script command, which sets whether the patterns are shown.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136768
     * @ghidraAddress PAL: 0x00137f80
     */
    static void OnShowPatterns(DataArray *pCommand, void *pUserData);

    /**
     * Run the "burn_patterns" script command, which sets whether the patterns are burnt.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x001367a0
     * @ghidraAddress PAL: 0x00137fb8
     */
    static void OnBurnPatterns(DataArray *pCommand, void *pUserData);

    /**
     * Run the "enable_menu" script command, which enables or disables the menu.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x001367d8
     * @ghidraAddress PAL: 0x00137ff0
     */
    static void OnEnableMenu(DataArray *pCommand, void *pUserData);

    /**
     * Run the "allow_tempo_change" script command, which sets whether tempo changes are allowed.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136810
     * @ghidraAddress PAL: 0x00138028
     */
    static void OnAllowTempoChange(DataArray *pCommand, void *pUserData);

    /**
     * Run the "tempo_open" script command, which opens or closes the tempo menu.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136848
     * @ghidraAddress PAL: 0x00138060
     */
    static void OnTempoOpen(DataArray *pCommand, void *pUserData);

    /**
     * Run the "adjust_tempo" script command, which adjusts the tempo by its argument.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x00136880
     * @ghidraAddress PAL: 0x00138098
     */
    static void OnAdjustTempo(DataArray *pCommand, void *pUserData);

    /**
     * Run the "allow_pattern_cancel" script command, which passes its argument to
     * AllowPatternCancel().
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x001368b8
     * @ghidraAddress PAL: 0x001380d0
     */
    static void OnAllowPatternCancel(DataArray *pCommand, void *pUserData);

    /**
     * Run the "allow_rotation_left" script command, which sets whether rotation to the left is
     * allowed.
     *
     * @param pCommand The command.
     * @param pUserData The tutorial.
     * @ghidraAddress NTSC-U/C: 0x001368f0
     * @ghidraAddress PAL: 0x00138108
     */
    static void OnAllowRotationLeft(DataArray *pCommand, void *pUserData);

    SerialTasks mScript;        /*!< The steps, in order. */
    StreamQueue mStreams;       /*!< The narration streams. */
    bool mPitchingEnabled;      /*!< Cleared by "disable_pitching". */
    bool mRotationEnabled;      /*!< Cleared by "disable_rot". */
    bool mSectionChangeEnabled; /*!< Cleared by "disable_section_change". */
    bool mHudEnabled;           /*!< Cleared by "disable_hud". */
    bool mErasingEnabled;       /*!< Cleared by "disable_erasing". */
    int mReserved5C;            // +0x5c, cleared by the constructor and not yet identified.
    int mReserved60;            // +0x60, cleared by the constructor and not yet identified.
    int mReserved64;            // +0x64, cleared by the constructor and not yet identified.
    int mReserved68;            // +0x68, cleared by the constructor and not yet identified.
    bool mBurnPatterns;         /*!< Set by "burn_patterns". */
    bool mShowPatterns;         /*!< Set by "show_patterns". */
    bool mTempoChangeAllowed;   /*!< Set by "allow_tempo_change". */
    bool mLoopingAllowed;       /*!< Set by "allow_looping". */
    bool mPaused;               /*!< The tutorial is paused. */
    int mLastRotation;          /*!< The rotation NotifyRotate() last recorded, initially -1. */
    bool mRotationLeftAllowed;  /*!< Set by "allow_rotation_left", initially true. */
    RemixLogic *mLogic;         /*!< The remix session the tutorial drives. */

private:
    /**
     * Build the task for one script step.
     *
     * A "wait_stream" step waits for a narration stream, a "wait_time" step waits for a time, and
     * a "wait_interactive" step waits for the player. Any other step runs as a script command.
     *
     * @param pStep The step.
     * @return The task.
     * @ghidraAddress NTSC-U/C: 0x00137078
     * @ghidraAddress PAL: 0x001388d8
     */
    Task *CreateTask(DataArray *pStep);
};

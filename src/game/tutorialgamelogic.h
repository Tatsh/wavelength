#pragma once

#include <vector>

#include "game/btnevent.h"
#include "game/catchtrack.h"
#include "game/gamelogic.h"
#include "game/player.h"
#include "game/playnoteevent.h"
#include "game/playscriptcmd.h"
#include "game/rotateevent.h"
#include "game/song.h"
#include "game/streamqueue.h"
#include "game/track.h"
#include "met/metagame.h"
#include "msg/message.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/serialtasks.h"
#include "os/task.h"
#include "os/taskdonenotifier.h"
#include "script/dataarray.h"

/**
 * Rules of the tutorial song.
 *
 * The RTTI records the class as deriving from GameLogic and from TaskDoneNotifier, the second base
 * after the members of GameLogic. Game builds the logic while GameDb::mTutorial is set. The
 * "script" entry of the song's configuration lists the steps, each a task that mScript runs in
 * order once the song starts. The script commands the constructor registers drive the song: they
 * play narration streams, run the tracks named "SCRIPT" of the MIDI file, loop the song, give bars
 * of the catch tracks to the player, and switch the controls on and off. The logic saves the game
 * settings and the first player's controller bindings it changes and restores them when it is
 * destroyed. The end of the last step shows the end-of-tutorial menu.
 */
class TutorialGameLogic : public GameLogic, public TaskDoneNotifier {
public:
    /** Values of mRotateMode, the `disable_rot` argument. */
    enum RotateMode {
        kRotateModeNextOnly = 100,     /*!< Only rotations to the next track pass. */
        kRotateModePreviousOnly = 101, /*!< Only rotations to the previous track pass. */
        kRotateModeNone = 102,         /*!< No rotation passes. */
        kRotateModeAll = 103,          /*!< Every rotation passes. */
    };

    /**
     * Construct the logic of the tutorial song with a random seed.
     *
     * Registers the script commands, builds the steps, saves the game settings the commands
     * change, and switches the first player to the default controller bindings.
     *
     * @param pSong The song.
     * @param pSongConfig The entry of the song in the "songs" section.
     * @ghidraAddress NTSC-U/C: 0x0013f7e8
     */
    TutorialGameLogic(Song *pSong, DataArray *pSongConfig);

    /**
     * Cancel the scheduled deactivations, restore the song speed, the game settings, and the
     * controller bindings, and remove the script commands.
     *
     * @ghidraAddress NTSC-U/C: 0x0013fcb0
     * @ghidraAddress PAL: 0x00141660
     */
    ~TutorialGameLogic() override;

    /**
     * Report the type of a message and pass it to WorldLogic::DispatchPriv().
     *
     * @param pMsg The message.
     * @return The result of WorldLogic::DispatchPriv().
     * @ghidraAddress NTSC-U/C: 0x00141168
     * @ghidraAddress PAL: 0x00142b28
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the song position in ticks.
     *
     * @return The tick of the song scheduler's clock.
     * @ghidraAddress NTSC-U/C: 0x00140230
     * @ghidraAddress PAL: 0x00141bf0
     */
    int GetTick() override;

    /**
     * Report the song position on the song clock.
     *
     * @return The time of the song scheduler's clock.
     * @ghidraAddress NTSC-U/C: 0x00140250
     * @ghidraAddress PAL: 0x00141c10
     */
    float GetTime() override;

    /**
     * Advance the song, the steps, and the narration.
     *
     * Once the last step is done, the steps stop and the end-of-tutorial menu shows.
     *
     * @ghidraAddress NTSC-U/C: 0x00140270
     * @ghidraAddress PAL: 0x00141c30
     */
    void Poll() override;

    /**
     * Report how far the tutorial has played, from the span `set_songpos_params` sets.
     *
     * @return The fraction, limited to the range 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001401a8
     * @ghidraAddress PAL: 0x00141b68
     */
    float GetProgress() override;

    using GameLogic::HandleInput;

    /**
     * Pass a rotation that mRotateMode allows, unless the input is disabled.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00140028
     * @ghidraAddress PAL: 0x001419e8
     */
    void HandleInput(const RotateEvent &event) override;

    /**
     * Pass a note, unless the input is disabled.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001400c8
     * @ghidraAddress PAL: 0x00141a88
     */
    void HandleInput(const PlayNoteEvent &event) override;

    /**
     * Ignore the event. The tutorial does not change the display option.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00140128
     */
    void HandleInput(const BtnEvent<4> &event) override;

    /**
     * Deploy a power-up, unless the input or the power-ups are disabled.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001400f0
     * @ghidraAddress PAL: 0x00141ab0
     */
    void HandleInput(const BtnEvent<5> &event) override;

    /**
     * Pause or resume the song and the narration, and show the pause menu while paused.
     *
     * @param bPaused Whether to pause.
     * @param nPad The controller that requested the change.
     * @param nReason Non-zero when a disconnected controller paused the song.
     * @ghidraAddress NTSC-U/C: 0x001403e0
     * @ghidraAddress PAL: 0x00141da0
     */
    void SetPaused(bool bPaused, int nPad, int nReason) override;

    /**
     * Enable the first catch tracks and start the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x001402f8
     * @ghidraAddress PAL: 0x00141cb8
     */
    void OnStart() override;

    /**
     * Ignore the end of the song.
     *
     * @param bWon Whether the song was won.
     * @ghidraAddress NTSC-U/C: 0x00343af8
     */
    void OnFinish([[maybe_unused]] bool bWon) override {
    }

    /**
     * Record the bar, unless it precedes bar 0.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x00140328
     */
    void OnBar(int nBar) override;

    /**
     * Ignore the start of a section.
     *
     * @ghidraAddress NTSC-U/C: 0x00343ae8
     */
    void OnSection() override {
    }

    /**
     * Enable the next catch tracks, unless `set_manual_enable_next` asked for the script to do it.
     *
     * @param pTrack The track the phrase was captured on.
     * @param pPlayer The player that captured it.
     * @param nBar The first bar the capture clears.
     * @param bStreak Whether the capture continues the streak of the player.
     * @ghidraAddress NTSC-U/C: 0x00140338
     * @ghidraAddress PAL: 0x00141cf8
     */
    void OnPhraseCaptured(CatchTrack *pTrack, Player *pPlayer, int nBar, bool bStreak) override;

    /**
     * Ignore the end of a phrase.
     *
     * @param pTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001403d8
     */
    void OnPhraseEnded(Track *pTrack) override;

    /**
     * Ignore a missed phrase.
     *
     * @param pTrack The track the phrase was missed on.
     * @ghidraAddress NTSC-U/C: 0x00343af0
     */
    void OnPhraseMissed([[maybe_unused]] Track *pTrack) override {
    }

    /**
     * Arrange for a task to learn when the commands of a track named "SCRIPT" ran out.
     *
     * A track that no `midi` command started is reported done at once.
     *
     * @param pTask The task waiting for the track.
     * @param pszName The name of the commands of the track.
     * @ghidraAddress NTSC-U/C: 0x00140798
     * @ghidraAddress PAL: 0x00142158
     */
    void NotifyWhenDone(Task *pTask, const char *pszName) override;

    /**
     * Report whether no phrase of the first player's track plays at or after the current bar.
     *
     * The name is inferred.
     *
     * @return True when the track has no phrase left.
     * @ghidraAddress NTSC-U/C: 0x00140f08
     * @ghidraAddress PAL: 0x001428c8
     */
    bool IsPhraseAbsent();

    /**
     * Start the commands of a track named "SCRIPT" from the current tick.
     *
     * @param pszName The name of the commands of the track.
     * @ghidraAddress NTSC-U/C: 0x00140518
     * @ghidraAddress PAL: 0x00141ed8
     */
    void PlayScriptTrack(const char *pszName);

    /**
     * Queue a narration stream.
     *
     * @param pszName The stream name.
     * @param bSkipIfBusy Drop the request when a stream is already queued.
     * @ghidraAddress NTSC-U/C: 0x00140830
     * @ghidraAddress PAL: 0x001421f0
     */
    void QueueStream(const char *pszName, bool bSkipIfBusy);

    /**
     * Loop a range of bars of the song from a number of bars after the recorded bar.
     *
     * @param nStartBar The first bar of the range as written.
     * @param nNumBars The length of the range in bars.
     * @param nBarsAhead The bars after mBar the loop plays from.
     * @ghidraAddress NTSC-U/C: 0x00140850
     * @ghidraAddress PAL: 0x00142210
     */
    void LoopTrack(int nStartBar, int nNumBars, int nBarsAhead);

    /**
     * Disable or enable the input.
     *
     * @param bDisabled Disable the input.
     * @ghidraAddress NTSC-U/C: 0x001408d0
     * @ghidraAddress PAL: 0x00142298
     */
    void SetInputDisabled(bool bDisabled);

    /**
     * Set the rotations that pass.
     *
     * @param nMode One of RotateMode.
     * @ghidraAddress NTSC-U/C: 0x001408d8
     */
    void SetRotateMode(int nMode);

    /**
     * Disable or enable the power-ups.
     *
     * @param bDisabled Disable the power-ups.
     * @ghidraAddress NTSC-U/C: 0x001408e0
     */
    void SetPowerupsDisabled(bool bDisabled);

    /**
     * Give a run of bars of a catch track to the first player.
     *
     * @param nBar The first bar.
     * @param nBars The number of bars.
     * @param nTrack The catch track.
     * @ghidraAddress NTSC-U/C: 0x001408e8
     * @ghidraAddress PAL: 0x001422a8
     */
    void ActivateBars(int nBar, int nBars, int nTrack);

    /**
     * Schedule the deactivation of a run of bars of the first player's track.
     *
     * The deactivation runs at the start of the bar a number of bars after the next one.
     *
     * @param nBarsAhead The bars after the next bar the deactivation runs at.
     * @param nBar The first bar to deactivate.
     * @param nBars The number of bars to deactivate.
     * @ghidraAddress NTSC-U/C: 0x00140918
     * @ghidraAddress PAL: 0x001422d8
     */
    void ScheduleDeactivateTrack(int nBarsAhead, int nBar, int nBars);

    /**
     * Capture the phrase of the recorded bar on the first player's track.
     *
     * @ghidraAddress NTSC-U/C: 0x00140bb0
     * @ghidraAddress PAL: 0x00142570
     */
    void CaptureTrack();

    /**
     * Capture the phrase of the current bar on every catch track for the first player.
     *
     * @ghidraAddress NTSC-U/C: 0x00140c00
     * @ghidraAddress PAL: 0x001425c0
     */
    void CaptureAllTracks();

    /**
     * Place a power-up in a bar of a catch track.
     *
     * @param nPowerup One of GameLogic::Powerup.
     * @param nTrack The catch track.
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x00140ca0
     * @ghidraAddress PAL: 0x00142660
     */
    void PlacePowerup(int nPowerup, int nTrack, int nBar);

    /**
     * Move the first player to a catch track.
     *
     * @param nTrack The catch track.
     * @ghidraAddress NTSC-U/C: 0x00140cd0
     * @ghidraAddress PAL: 0x00142690
     */
    void MoveTrack(int nTrack);

    /**
     * Turn on or off the automatic play of the first player's track.
     *
     * @param bAutopilot Play the gems automatically.
     * @ghidraAddress NTSC-U/C: 0x00140cf8
     */
    void SetAutopilot(bool bAutopilot);

    /**
     * Hide or show the seeker of the first player's track.
     *
     * @param bNoSeeker Hide the seeker.
     * @ghidraAddress NTSC-U/C: 0x00140d50
     */
    void SetNoSeeker(bool bNoSeeker);

    /**
     * Leave the enabling of the next catch tracks to the script, or return it to the captures.
     *
     * @param bManual Leave the enabling to the script.
     * @ghidraAddress NTSC-U/C: 0x00140da8
     * @ghidraAddress PAL: 0x001422a0
     */
    void SetManualEnableNext(bool bManual);

    /**
     * Set the span GetProgress() measures.
     *
     * @param nPlayedBars The bars the span counts as played at the current tick.
     * @param nTotalBars The length of the span in bars.
     * @ghidraAddress NTSC-U/C: 0x00140130
     * @ghidraAddress PAL: 0x00141af0
     */
    void SetSongPosParams(int nPlayedBars, int nTotalBars);

    /**
     * Point at the next phrase of a catch track from a number of bars after the clock's bar.
     *
     * @param nTrack The catch track.
     * @param nBarsAhead The bars after the clock's bar to search from.
     * @param nUnused The third argument of the command. The body does not read it.
     * @ghidraAddress NTSC-U/C: 0x00140f80
     * @ghidraAddress PAL: 0x00142940
     */
    void ShowStreakArrow(int nTrack, int nBarsAhead, int nUnused);

    /**
     * Schedule the first player's energy meter to change at the start of a later bar.
     *
     * @param nBarsAhead The bars after the current bar.
     * @param fJuice The energy, in percent.
     * @ghidraAddress NTSC-U/C: 0x00140ff8
     * @ghidraAddress PAL: 0x001429b8
     */
    void ScheduleSetJuice(int nBarsAhead, float fJuice);

    /**
     * Schedule the stage completion display at the start of a later bar.
     *
     * @param nBarsAhead The bars after the current bar.
     * @ghidraAddress NTSC-U/C: 0x001410b8
     * @ghidraAddress PAL: 0x00142a78
     */
    void ScheduleStageComplete(int nBarsAhead);

    /**
     * Enable the next catch tracks from the bar after the current one.
     *
     * @ghidraAddress NTSC-U/C: 0x00140390
     * @ghidraAddress PAL: 0x00141d50
     */
    void EnableNext();

    SerialTasks mScript;                          /*!< The steps, in order. */
    StreamQueue mStreams;                         /*!< The narration streams. */
    std::vector<Ptr<PlayScriptCmd> > mScriptCmds; /*!< The tracks `midi` started. +0x504 */
    int mBar;                                     /*!< The bar OnBar() last recorded. +0x514 */
    int mRotateMode;        /*!< One of RotateMode, initially kRotateModeNone. +0x518 */
    bool mInputDisabled;    /*!< Set by `disable_input`. +0x51c */
    float mProgressLength;  /*!< The span GetProgress() measures, in ticks. +0x520 */
    float mProgressOffset;  /*!< Added to the tick GetProgress() measures. +0x524 */
    bool mPowerupsDisabled; /*!< Set by `disable_powerups`, initially true. +0x528 */
    bool mManualEnableNext; /*!< Set by `set_manual_enable_next`. +0x52c */
    std::vector<Ptr<Command> > mDeactivators; /*!< The scheduled deactivations. +0x530 */
    bool mSavedNoCapture;      /*!< GameConfig::mNoCapture before the tutorial. +0x540 */
    bool mSavedNoDeactivate;   /*!< GameConfig::mNoDeactivate before the tutorial. +0x544 */
    int mReserved548;          // +0x548, not read or written by the class.
    bool mSavedStreaksEnabled; /*!< GameConfig::mStreaksEnabled before the tutorial. +0x54c */
    bool mSavedGuideTicks;     /*!< GameConfig::mGuideTicks before the tutorial. +0x550 */
    int mSavedStrandBars;      /*!< The strand bars of the player count. +0x554 */

private:
    /**
     * Build the task for one script step.
     *
     * A "wait_stream" step waits for a narration stream, a "wait_midi" step for the end of a track
     * named "SCRIPT", a "wait_time" step for a time, and a "wait_interactive" step for the player.
     * Any other step runs as a script command.
     *
     * @param pStep The step.
     * @return The task.
     * @ghidraAddress NTSC-U/C: 0x00140db0
     * @ghidraAddress PAL: 0x00142770
     */
    Task *CreateTask(DataArray *pStep);

    /**
     * Report the choice of the pause menu or the end-of-tutorial menu.
     *
     * @param action The choice.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x00140488
     * @ghidraAddress PAL: 0x00141e48
     */
    static void OnDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Run the `midi` script command, which starts a track named "SCRIPT".
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f0f8
     * @ghidraAddress PAL: 0x001409c8
     */
    static void OnMidiCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `sched_deactivate_track` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f130
     * @ghidraAddress PAL: 0x00140a00
     */
    static void OnSchedDeactivateTrackCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `stream` script command, which queues a narration stream.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f1a8
     * @ghidraAddress PAL: 0x00140a78
     */
    static void OnStreamCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `loop_track` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f228
     * @ghidraAddress PAL: 0x00140af8
     */
    static void OnLoopTrackCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `disable_rot` script command, which sets mRotateMode.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f2a0
     * @ghidraAddress PAL: 0x00140b70
     */
    static void OnDisableRotCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `disable_input` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f2d8
     * @ghidraAddress PAL: 0x00140ba8
     */
    static void OnDisableInputCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `tutorial_print` script command, which reads its text and does nothing with it.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f310
     * @ghidraAddress PAL: 0x00140be0
     */
    static void OnTutorialPrintCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `disable_powerups` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f330
     * @ghidraAddress PAL: 0x00140c00
     */
    static void OnDisablePowerupsCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `activate_bars` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f368
     * @ghidraAddress PAL: 0x00140c38
     */
    static void OnActivateBarsCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `set_songpos_params` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f3e0
     * @ghidraAddress PAL: 0x00140cb0
     */
    static void OnSetSongPosParamsCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `capture_track` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f440
     * @ghidraAddress PAL: 0x00140d10
     */
    static void OnCaptureTrackCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `capture_all_tracks` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f460
     * @ghidraAddress PAL: 0x00140d30
     */
    static void OnCaptureAllTracksCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `set_powerup` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f480
     * @ghidraAddress PAL: 0x00140d50
     */
    static void OnSetPowerupCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `move_track` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f4f8
     * @ghidraAddress PAL: 0x00140dc8
     */
    static void OnMoveTrackCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `set_no_capture` script command, which sets GameConfig::mNoCapture.
     *
     * @param pCommand The command.
     * @param pUserData Null.
     * @ghidraAddress NTSC-U/C: 0x0013f530
     * @ghidraAddress PAL: 0x00140e00
     */
    static void OnSetNoCaptureCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `set_no_deactivate` script command, which sets GameConfig::mNoDeactivate.
     *
     * @param pCommand The command.
     * @param pUserData Null.
     * @ghidraAddress NTSC-U/C: 0x0013f560
     * @ghidraAddress PAL: 0x00140e30
     */
    static void OnSetNoDeactivateCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `set_no_seeker` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f590
     * @ghidraAddress PAL: 0x00140e60
     */
    static void OnSetNoSeekerCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `set_no_streaks` script command, which clears GameConfig::mStreaksEnabled.
     *
     * @param pCommand The command.
     * @param pUserData Null.
     * @ghidraAddress NTSC-U/C: 0x0013f5c8
     * @ghidraAddress PAL: 0x00140e98
     */
    static void OnSetNoStreaksCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `set_strand_bars` script command for the number of players.
     *
     * @param pCommand The command.
     * @param pUserData Null.
     * @ghidraAddress NTSC-U/C: 0x0013f5f8
     * @ghidraAddress PAL: 0x00140ec8
     */
    static void OnSetStrandBarsCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `show_streak_arrow` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f648
     * @ghidraAddress PAL: 0x00140f18
     */
    static void OnShowStreakArrowCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `sched_set_juice` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f6c0
     * @ghidraAddress PAL: 0x00140f90
     */
    static void OnSchedSetJuiceCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `set_autopilot` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f720
     * @ghidraAddress PAL: 0x00140ff0
     */
    static void OnSetAutopilotCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `set_manual_enable_next` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f758
     * @ghidraAddress PAL: 0x00141028
     */
    static void OnSetManualEnableNextCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `enable_next` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f790
     * @ghidraAddress PAL: 0x00141060
     */
    static void OnEnableNextCommand(DataArray *pCommand, void *pUserData);

    /**
     * Run the `sched_stage_complete` script command.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013f7b0
     * @ghidraAddress PAL: 0x00141080
     */
    static void OnSchedStageCompleteCommand(DataArray *pCommand, void *pUserData);
};

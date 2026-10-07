#pragma once

#include "game/catchtrack.h"
#include "game/gamelogic.h"
#include "game/juicemeter.h"
#include "game/player.h"
#include "game/song.h"
#include "game/track.h"
#include "met/metagame.h"
#include "msg/message.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Rules of a song played by one player against the juice meter.
 *
 * The RTTI records the class as deriving from GameLogic. Each captured phrase adds juice, and
 * each bar without a capture drains it. The song is lost when the juice runs out while a phrase
 * plays, and won at the end of the last section. A won song that completes a campaign tier runs
 * the campaign win sequence, and every won song loops on the freestyle track until the player
 * leaves.
 */
class SoloGameLogic : public GameLogic {
public:
    /** Steps of the campaign win sequence, the values of mWinSequenceStep. */
    enum WinSequenceStep {
        kWinStepCampaign = 0,       /*!< The campaign completion message. */
        kWinStepNextDifficulty = 1, /*!< The message that unlocks the next difficulty. */
        kWinStepNextCampaign = 2,   /*!< The third campaign message. */
        kWinStepFinalCampaign = 3,  /*!< The fourth campaign message. */
        kWinStepPrompt = 4,         /*!< The prompt to press start. */
        kWinStepDone = 5,           /*!< Every step ran. */
    };

    /**
     * Command that shows the stage completion message of the current section.
     *
     * The RTTI records the class as nested in SoloGameLogic and as deriving from Command.
     */
    class CheckpointTextCmd : public Command {
    public:
        /**
         * Construct the command for a logic.
         *
         * @param pLogic The logic.
         */
        explicit CheckpointTextCmd(SoloGameLogic *pLogic) : mLogic(pLogic) {
        }

        /**
         * Release the command.
         *
         * @ghidraAddress NTSC-U/C: 0x00340ec8
         * @ghidraAddress PAL: 0x003ae400
         */
        ~CheckpointTextCmd() override {
        }

        /**
         * Show the localised "STAGE_COMPLETED" message with the number of the section.
         *
         * @ghidraAddress NTSC-U/C: 0x00340f40
         * @ghidraAddress PAL: 0x003ae478
         */
        void Execute() override;

        SoloGameLogic *mLogic; /*!< The logic. */
    };

    /**
     * Construct the logic of a solo song.
     *
     * Reads the sound banks from the "game" section of the configuration, totals the bars the
     * player plays, and registers the script commands "juice", "win_cheat", and "win_sequence".
     *
     * @param pSong The song.
     * @param pConfig The configuration of the logic.
     * @param nSeed The seed of the random numbers.
     * @ghidraAddress NTSC-U/C: 0x00139140
     * @ghidraAddress PAL: 0x0013a9a0
     */
    SoloGameLogic(Song *pSong, DataArray *pConfig, int nSeed);

    /**
     * Unregister the script commands and restore the song speed.
     *
     * @ghidraAddress NTSC-U/C: 0x00139550
     * @ghidraAddress PAL: 0x0013ae18
     */
    ~SoloGameLogic() override;

    /**
     * Handle a message sent to the logic.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x0013b2b0
     * @ghidraAddress PAL: 0x0013cb80
     */
    void DispatchPriv(Message *pMsg) override;

    /**
     * Report the game tick, the song tick shifted by the restarts of the victory lap.
     *
     * @return The tick.
     * @ghidraAddress NTSC-U/C: 0x00139b18
     * @ghidraAddress PAL: 0x0013b3e0
     */
    int GetTick() override;

    /**
     * Report the game time, the song time shifted by the restarts of the victory lap.
     *
     * @return The time, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00139b50
     * @ghidraAddress PAL: 0x0013b418
     */
    float GetTime() override;

    using GameLogic::HandleInput;

    /**
     * Pass a played note on unless a dialog is open and the button is down.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00139c80
     * @ghidraAddress PAL: 0x0013b548
     */
    void HandleInput(const PlayNoteEvent &event) override;

    /**
     * Act on a press of the start button, ending the victory lap or the win sequence.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00139b88
     * @ghidraAddress PAL: 0x0013b450
     */
    void HandleInput(const BtnEvent<3> &event) override;

    /**
     * Pass a stick event on unless a dialog is open.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00139cb8
     */
    void HandleInput(const StickEvent<2> &event) override;

    /**
     * Pass a stick event on unless a dialog is open.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00139ce0
     */
    void HandleInput(const StickEvent<6> &event) override;

    /**
     * Pause the playing song and show the pause dialog, or resume it.
     *
     * @param bPaused Pause the song.
     * @param nPad The controller that paused the song, or -1.
     * @param nReason Non-zero when a disconnected controller paused the song.
     * @ghidraAddress NTSC-U/C: 0x001399b0
     * @ghidraAddress PAL: 0x0013b278
     */
    void SetPaused(bool bPaused, int nPad, int nReason) override;

    /**
     * Deploy the freestyle power-up and count the bars it makes possible to capture.
     *
     * @param pPlayer The player.
     * @return True when the power-up was deployed.
     * @ghidraAddress NTSC-U/C: 0x0013a6f8
     * @ghidraAddress PAL: 0x0013bfc8
     */
    bool DeployFreestyle(Player *pPlayer) override;

    /**
     * Fill the juice meter, show the controller display, and enable the first tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x00139658
     * @ghidraAddress PAL: 0x0013af20
     */
    void OnStart() override;

    /**
     * Record the result of the song and start the victory lap or the loss.
     *
     * @param bWon The player won.
     * @ghidraAddress NTSC-U/C: 0x0013a958
     * @ghidraAddress PAL: 0x0013c228
     */
    void OnFinish(bool bWon) override;

    /**
     * Drain the juice for the bar and end the song when the juice ran out.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x0013a008
     * @ghidraAddress PAL: 0x0013b8d0
     */
    void OnBar(int nBar) override;

    /**
     * Win the song after the last section, or award the energy bonus of the section.
     *
     * @ghidraAddress NTSC-U/C: 0x0013a2b0
     * @ghidraAddress PAL: 0x0013bb80
     */
    void OnSection() override;

    /**
     * Add the juice of a capture and count the bars the capture makes possible to capture.
     *
     * @param pTrack The track of the phrase.
     * @param pPlayer The player who captured the phrase.
     * @param nBar The bar after the phrase.
     * @param bStreak The capture continued a streak.
     * @ghidraAddress NTSC-U/C: 0x00139d08
     * @ghidraAddress PAL: 0x0013b5d0
     */
    void OnPhraseCaptured(CatchTrack *pTrack, Player *pPlayer, int nBar, bool bStreak) override;

    /**
     * Uncount the captured track.
     *
     * @param pTrack The track of the phrase.
     * @ghidraAddress NTSC-U/C: 0x00139e98
     */
    void OnPhraseEnded(Track *pTrack) override;

    /**
     * Lose the song when the phrase missed was the last chance, and commit the pending juice.
     *
     * @param pTrack The track of the phrase.
     * @ghidraAddress NTSC-U/C: 0x00139ea8
     * @ghidraAddress PAL: 0x0013b770
     */
    void OnPhraseMissed(Track *pTrack) override;

    /**
     * Show the next message of the campaign win sequence and schedule the one after.
     *
     * @ghidraAddress NTSC-U/C: 0x00139740
     * @ghidraAddress PAL: 0x0013b008
     */
    void AdvanceWinSequence();

    /**
     * Add the pending juice to the meter unless a player is freestyling.
     *
     * @ghidraAddress NTSC-U/C: 0x00139f18
     * @ghidraAddress PAL: 0x0013b7e0
     */
    void CommitPendingJuice();

    /**
     * Take one unit from the pending juice for a bar of a phrase the player does not play.
     *
     * @ghidraAddress NTSC-U/C: 0x00139f78
     * @ghidraAddress PAL: 0x0013b840
     */
    void DrainPendingJuice();

    /**
     * Win the song at once with a fixed score.
     *
     * @ghidraAddress NTSC-U/C: 0x0013a848
     * @ghidraAddress PAL: 0x0013c118
     */
    void WinNow();

    /**
     * Show the result dialog once the display and the win sound bank are ready, or retry later.
     *
     * @ghidraAddress NTSC-U/C: 0x0013a8a8
     * @ghidraAddress PAL: 0x0013c178
     */
    void SaveOrRetry();

    /**
     * Stop the song clock and the sound at the end of the slowdown of a lost song.
     *
     * @ghidraAddress NTSC-U/C: 0x0013adb8
     * @ghidraAddress PAL: 0x0013c688
     */
    void FreezeAfterLoss();

    /**
     * Start the victory lap on the freestyle track.
     *
     * @param bIntro Run the intro of the display before the lap starts.
     * @ghidraAddress NTSC-U/C: 0x0013ae50
     * @ghidraAddress PAL: 0x0013c720
     */
    void ResumeWithCue(bool bIntro);

    /**
     * Restart the song from the start of the next loop of the victory lap.
     *
     * @param bRestartIntro Run the intro of the display again.
     * @ghidraAddress NTSC-U/C: 0x0013b020
     * @ghidraAddress PAL: 0x0013c8f0
     */
    void Restart(bool bRestartIntro);

    /**
     * Act on the choice of the pause dialog.
     *
     * @param action The choice.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x00139a88
     * @ghidraAddress PAL: 0x0013b350
     */
    static void OnPauseDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Act on the choice of the dialog of a won song.
     *
     * @param action The choice.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013a4b0
     * @ghidraAddress PAL: 0x0013bd80
     */
    static void OnWinDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Act on the choice of the dialog of a lost song or of a practice song.
     *
     * @param action The choice.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013a640
     * @ghidraAddress PAL: 0x0013bf10
     */
    static void OnResultDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Run the "juice" script command, which adds its argument to the juice meter.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013a7b0
     * @ghidraAddress PAL: 0x0013c080
     */
    static void OnJuice(DataArray *pCommand, void *pUserData);

    /**
     * Run the "win_cheat" script command, which wins the song.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013a7f0
     * @ghidraAddress PAL: 0x0013c0c0
     */
    static void OnWinCheat(DataArray *pCommand, void *pUserData);

    /**
     * Run the "win_sequence" script command, which runs the win sequence after the next win.
     *
     * @param pCommand The command.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x0013a810
     * @ghidraAddress PAL: 0x0013c0e0
     */
    static void OnWinSequence(DataArray *pCommand, void *pUserData);

    JuiceMeter mJuice;                         /*!< The juice of the player. */
    Player *mPlayer;                           /*!< The player, the first local player. */
    int mTickOffset;                           /*!< GetTick() minus the song tick. */
    float mTimeOffset;                         /*!< GetTime() minus the song time. */
    int mBankSlot;                             /*!< `game_fx_bank_slot`, the sound bank slot. */
    String mSoloBankFile;                      /*!< `solo_game_fx_bank_file`. */
    String mWinBankFile;                       /*!< `win_fx_bank_file`. */
    Ptr<Command> mFreezeCmd;                   /*!< Runs FreezeAfterLoss(). */
    Ptr<Command> mRestartCmd;                  /*!< Runs Restart() with true. */
    Ptr<Command> mIntroCmd;                    /*!< Runs GfxManager::StartIntro(). */
    Ptr<Command> mSaveOrRetryCmd;              /*!< Runs SaveOrRetry(). */
    Ptr<CheckpointTextCmd> mCheckpointTextCmd; /*!< Shows the stage completion message. */
    Ptr<Command> mSwapMovieCmd;                /*!< Alternates the winner movie each bar. */
    float mIntroDuration;                      /*!< The duration of the display intro. */
    int mPlayBars;                             /*!< The bars of the sections after checkpoints. */
    int mCapturedTracks;                       /*!< The tracks with a captured phrase playing. */
    int mFailingTrack;                         /*!< The track whose phrase is the last chance. */
    float mPendingJuice;                       /*!< The juice not yet added to the meter. */
    float mCaptureJuice;                       /*!< The juice of one capture. */
    int mDialogOpen;                           /*!< Non-zero while a result dialog is open. */
    int mForceWinSequence;                     /*!< Non-zero after "win_sequence" ran. */
    int mWinSequenceStep;                      /*!< One of WinSequenceStep. */
    int mFullMixBars;                          /*!< The bars played with every track captured. */
    int mBestStreak;                           /*!< The longest streak. */
    int mPossibleCaptureBars;                  /*!< The bars it was possible to capture. */
};

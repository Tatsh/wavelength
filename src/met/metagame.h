#pragma once

#include <list>
#include <vector>

#include "app/msgsink.h"
#include "game/unlockableitem.h"
#include "met/gizmo.h"
#include "met/helppanel.h"
#include "met/metagamearena.h"
#include "met/metamusicsong.h"
#include "met/mix.h"
#include "msg/gameparamsupdatemsg.h"
#include "msg/joypadinputmsg.h"
#include "msg/launchpadabortedmsg.h"
#include "msg/lobbyconnectionlostmsg.h"
#include "msg/message.h"
#include "msg/shareremixprogressmsg.h"
#include "netflow/netchatroominfo.h"
#include "os/string.h"
#include "rnd/cam.h"
#include "rnd/environ.h"
#include "rnd/rndloader.h"
#include "synth/syntheffects.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uiscreenchangemsg.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Front end and flow of the whole game, from the start-up screens through a song and back.
 *
 * The RTTI includes the name and records MsgSink as the one base. The vptr sits at offset 0. The
 * one instance is TheMetagame.
 *
 * The main loop drives it together with WorldMgr. Update() reports what the front end requested,
 * and the main loop answers by moving WorldMgr and calling the matching Enter routine here.
 *
 * At start-up the metagame walks through the stages of LoadStage. It loads the screen groups,
 * shows the memory card check and the presented-by screen, loads the sound banks, the arena
 * scene, and the menu music, plays the intro movie once the screens reach `blank`, and then moves
 * the front end to `pre_logo`, which leads to the title.
 */
class Metagame : public MsgSink {
public:
    /** Values of mState. */
    enum State {
        kStateFrontEnd = 0,   /*!< The front end screens are showing. */
        kStateLoading = 1,    /*!< A world is loading for a song. */
        kStatePlaying = 2,    /*!< The world is running. */
        kStateLeaving = 3,    /*!< The world ended, and the screens after a song are showing. */
        kStateRestarting = 4, /*!< The world is being unloaded to load again. */
    };

    /** Values Update() reports. */
    enum Event {
        kEventNone = 0,             /*!< Nothing was requested. */
        kEventStartGame = 1,        /*!< Load a world for the chosen song. */
        kEventReturnToFrontEnd = 2, /*!< Return from the screens after a song to the front end. */
        kEventLeaveGame = 3,        /*!< Abandon the running world. */
        kEventQuit = 4,             /*!< End the program. */
    };

    /** Values of mLoadStage, the progress of the front end through its loads. */
    enum LoadStage {
        kLoadStageAlways = 0,           /*!< The `always` screen group loads. */
        kLoadStageIntro = 1,            /*!< The `intro` screen group loads. */
        kLoadStageIntroAndFrontEnd = 2, /*!< The `intro` and `front_end` groups load together. */
        kLoadStageStartFxBanks = 3,     /*!< The sound effect banks are about to load. */
        kLoadStageFxBanks = 4,          /*!< The sound effect banks load. */
        kLoadStageArena = 5,            /*!< The arena scene loads. */
        kLoadStageMusic = 6,            /*!< The menu song is read. */
        kLoadStageMusicBanks = 7,       /*!< The music banks load, and the intro movie waits. */
        kLoadStageDone = 8,             /*!< Every load has finished. */
    };

    /** Kinds of dialog ShowDialog() shows over a song. */
    enum DialogType {
        kDialogSoloWon = 0,      /*!< The result of a won solo song, or of a practice song. */
        kDialogSoloLost = 1,     /*!< The result of a lost solo song. */
        kDialogEndGame = 2,      /*!< The results at the end of a multiplayer song. */
        kDialogPause = 5,        /*!< The pause menu. */
        kDialogNoController = 6, /*!< The pause menu of a disconnected controller. */
        kDialogTutorialEnd = 7,  /*!< The menu at the end of the tutorial. The name is inferred. */
    };

    /** The screens that follow a song, in the order QueueUnlocks() queues them. */
    enum UnlockEvent {
        kUnlockEventNone = 0,         /*!< No screen. */
        kUnlockEventBoss = 1,         /*!< `unlock_boss`, a boss song unlocked. */
        kUnlockEventParts = 2,        /*!< `unlock_parts`, avatar parts and emblems unlocked. */
        kUnlockEventSong = 3,         /*!< `song_decrypt`, a song unlocked. */
        kUnlockEventSaveFreq = 5,     /*!< `auto_save_freq`, the Freq changed. */
        kUnlockEventSaveSettings = 6, /*!< `auto_save_settings`, the settings changed. */
        kUnlockEventFreestyleTip = 7, /*!< `freestyle_lap_tip`, the first freestyle lap. */
    };

    /** Choices a dialog reports to its callback. */
    enum DialogAction {
        kDialogActionNone = 0,     /*!< No choice made yet. */
        kDialogActionQuit = 1,     /*!< Quit the song. */
        kDialogActionPractice = 2, /*!< Play the song again in practice mode. */
        kDialogActionResume = 3,   /*!< Return to the song. */
        kDialogActionEnd = 4,      /*!< End the song. */
        kDialogActionContinue = 5, /*!< Continue to the victory lap. */
    };

    /**
     * Routine a dialog reports the player's choice to.
     *
     * @param action The choice.
     * @param pUserData The value given to ShowDialog().
     */
    typedef void (*DialogCallback)(DialogAction action, void *pUserData);

    /**
     * Construct the metagame in kStateFrontEnd with nothing loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00163328
     * @ghidraAddress PAL: 0x001660a0
     */
    Metagame();

    /**
     * Destroy the metagame.
     *
     * @ghidraAddress NTSC-U/C: 0x00354d70
     * @ghidraAddress PAL: 0x003c1fc0
     */
    ~Metagame() override;

    /**
     * Load one of the four loading screens at random from `metagame\loading%d.rnd` and draw it.
     *
     * The main loop calls the routine before the other subsystems start, and the screen therefore
     * shows while they load. The screen is drawn into both frame buffers.
     *
     * @ghidraAddress NTSC-U/C: 0x00164500
     * @ghidraAddress PAL: 0x00167350
     */
    void ShowLoadingScreen();

    /**
     * Read the metagame configuration, start the front end, and register the script commands.
     *
     * The screen classes are registered, the screen file is read, and the `always` group starts
     * loading. The metagame listens to the front end, the controllers, and the keyboard.
     *
     * @ghidraAddress NTSC-U/C: 0x00164650
     * @ghidraAddress PAL: 0x001674f0
     */
    void Init();

    /**
     * Unregister the script commands and release what Init() and the loads created.
     *
     * @ghidraAddress NTSC-U/C: 0x00164a10
     * @ghidraAddress PAL: 0x001678c8
     */
    void Terminate();

    /**
     * Report mState.
     *
     * @return One of State.
     * @ghidraAddress NTSC-U/C: 0x00164b40
     * @ghidraAddress PAL: 0x00167a00
     */
    int GetState() const;

    /**
     * Report the display name of the arena chosen last.
     *
     * @return The localized name.
     * @ghidraAddress NTSC-U/C: 0x00164b48
     * @ghidraAddress PAL: 0x00167a08
     */
    const char *SelectedArenaName();

    /**
     * Report the display name of an arena.
     *
     * @param pszArena The arena token.
     * @return The localized name.
     * @ghidraAddress NTSC-U/C: 0x00164b68
     * @ghidraAddress PAL: 0x00167a28
     */
    const char *ArenaName(const char *pszArena);

    /**
     * Enter kStateFrontEnd.
     *
     * @ghidraAddress NTSC-U/C: 0x00164b90
     * @ghidraAddress PAL: 0x00167a50
     */
    void EnterFrontEnd();

    /**
     * Enter kStateLoading.
     *
     * @ghidraAddress NTSC-U/C: 0x00164c18
     * @ghidraAddress PAL: 0x00167ad8
     */
    void EnterLoading();

    /**
     * Enter kStatePlaying.
     *
     * @ghidraAddress NTSC-U/C: 0x00164c98
     * @ghidraAddress PAL: 0x00167b58
     */
    void EnterPlaying();

    /**
     * Enter kStateLeaving.
     *
     * @ghidraAddress NTSC-U/C: 0x00164cd0
     * @ghidraAddress PAL: 0x00167b90
     */
    void EnterLeaving();

    /**
     * Enter kStateRestarting.
     *
     * @ghidraAddress NTSC-U/C: 0x00164d10
     * @ghidraAddress PAL: 0x00167bd0
     */
    void EnterRestarting();

    /**
     * Advance the front end by one frame and report the request it made.
     *
     * The pending event is cleared as it is reported.
     *
     * @return One of Event.
     * @ghidraAddress NTSC-U/C: 0x00164d48
     * @ghidraAddress PAL: 0x00167c08
     */
    int Update();

    /**
     * Show the blank screen unless a dialog is showing.
     *
     * GameLogic::Start() schedules the routine shortly before the first bar. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00165048
     * @ghidraAddress PAL: 0x00167f08
     */
    void ShowBlankScreen();

    /**
     * Show a help text on the help panel.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x00165080
     * @ghidraAddress PAL: 0x00167f40
     */
    void SetHelpText(const char *pszText);

    /**
     * Show a text of the buttons on the help panel.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x001650a0
     * @ghidraAddress PAL: 0x00167f60
     */
    void SetActionText(const char *pszText);

    /**
     * Draw the part of the front end that goes under the world, the arena scene.
     *
     * @ghidraAddress NTSC-U/C: 0x001655e8
     * @ghidraAddress PAL: 0x001684f8
     */
    void Draw();

    /**
     * Draw the part of the front end that goes over the world.
     *
     * While the screen groups load, the loading screen fades out. Afterwards the front end camera,
     * the gizmo, the front end lights, and the screens are drawn.
     *
     * @ghidraAddress NTSC-U/C: 0x00165618
     * @ghidraAddress PAL: 0x00168528
     */
    void DrawOverlay();

    /**
     * Show the arenas the player has unlocked on the arena scene.
     *
     * @ghidraAddress NTSC-U/C: 0x00164630
     * @ghidraAddress PAL: 0x001674d0
     */
    void ShowUnlockedArenas();

    /**
     * Record the result of a campaign song in the profile of the first player.
     *
     * A remix, a practice song, and a demo record nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x00165fb8
     * @ghidraAddress PAL: 0x00168f80
     */
    void RecordCampaignResult();

    /**
     * Queue the unlock screens of the items the last song unlocked.
     *
     * @ghidraAddress NTSC-U/C: 0x001660c8
     * @ghidraAddress PAL: 0x00169090
     */
    void QueueUnlocks();

    /**
     * Show a dialog over the song and report the player's choice to a callback.
     *
     * @param type The dialog.
     * @param pfnCallback The routine the choice is reported to.
     * @param pUserData A value passed back to the callback unchanged.
     * @param nPad The controller the dialog listens to, or -1 for every controller.
     * @ghidraAddress NTSC-U/C: 0x00166840
     * @ghidraAddress PAL: 0x00169808
     */
    void ShowDialog(DialogType type, DialogCallback pfnCallback, void *pUserData, int nPad);

    /**
     * Show a dialog over the song that listens to every controller.
     *
     * A scheduled dialog command calls the routine through this form, which passes -1 for the
     * controller.
     *
     * @param type The dialog.
     * @param pfnCallback The routine the choice is reported to.
     * @param pUserData A value passed back to the callback unchanged.
     */
    void ShowDialogToAll(DialogType type, DialogCallback pfnCallback, void *pUserData) {
        ShowDialog(type, pfnCallback, pUserData, -1);
    }

    /**
     * Show the screens that follow a song, from its result to the queued unlocks.
     *
     * @param action The choice the result dialog reported.
     * @ghidraAddress NTSC-U/C: 0x00166cb8
     * @ghidraAddress PAL: 0x00169ca8
     */
    void ShowEndGameScreens(DialogAction action);

    /**
     * Show the next queued unlock screen, or finish the dialog once the queue is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00166e08
     * @ghidraAddress PAL: 0x00169df8
     */
    void AdvanceUnlocks();

    /**
     * Switch the menu music of the song screen to mix 7 over 1920 ticks while no song clip plays,
     * or to mix 9 over 500 ticks before a clip starts.
     *
     * @param bIdle Whether no song clip plays.
     * @ghidraAddress NTSC-U/C: 0x001692c0
     * @ghidraAddress PAL: 0x0016c448
     */
    void SetSongScreenMix(bool bIdle);

    /**
     * Handle a message sent to the metagame.
     *
     * @param pMsg The message.
     * @return The result of the handler of the message's type, or false for any other message.
     * @ghidraAddress NTSC-U/C: 0x00168038
     * @ghidraAddress PAL: 0x0016b0c8
     */
    bool DispatchPriv(Message *pMsg) override;

    int mState;                      /*!< One of State. */
    int mLoadStage;                  /*!< One of LoadStage. */
    RndLoader *mLoadingScreenLoader; /*!< The load of the loading screen, until the intro. */
    int mReloadFrontEnd;             /*!< Set when the front end loads again after a song. */
    int mEvent;                      /*!< The Event Update() reports next. */
    MetagameArena *mArena;           /*!< The arena scene. */
    HelpPanel *mHelpPanel;           /*!< The `help` panel. */
    int mLastSelectButton;           /*!< The JoypadButton that last chose a component. */
    float mTime;                     /*!< The front end clock, sped up while mSpeed is above 1. */
    float mTick;                     /*!< The tick of the front end scheduler. */
    float mLastSchedulerTime;        /*!< The scheduler time of the last Update(). */
    const char *mNextScreen;         /*!< The screen the front end moves to once it loads. */
    float mLeaveTime;                /*!< The system time a song's loading screen ends, or 0. */
    int mLeaveScreenShown;           /*!< Whether the screen after a song has been shown. */
    int mFirstBoot;                  /*!< Set until the first song, while the intro is due. */
    int mDialogShowing;              /*!< Whether a dialog shows over the song. */
    DialogCallback mDialogCallback;  /*!< The routine the dialog reports to, or null. */
    void *mDialogUserData;           /*!< The value passed back to mDialogCallback. */
    DialogAction mDialogAction;      /*!< The choice made in the dialog. */
    Mix *mMusic;                     /*!< The menu music, while the front end shows. */
    MetaMusicSong *mMusicSong;       /*!< The menu song of mMusic. */
    SynthEffects mEffects;           /*!< The `effects` entry of the metagame configuration. */
    float *mTickMs;                  /*!< The milliseconds of one tick of the front end clock. */
    int mSharedMusicFadedIn;         /*!< Whether the shared music has faded in. */
    int mGameFxBankSlot;             /*!< `game_fx_bank_slot` of the game configuration. */
    int mMetaFxBankSlot;             /*!< `meta_fx_bank_slot` of the metagame configuration. */
    int mMusicSharedBankSlot;        /*!< `music_shared_bank_slot` of the metagame configuration. */
    int mMusicSwapBankSlot;          /*!< `music_swap_bank_slot` of the metagame configuration. */
    NetChatroomInfo mChatroom;       /*!< The chat room this console joined last. +0xdc */
    String mSelectedArena;           /*!< The arena chosen last. */
    int mFreqsOnCard;                /*!< Non-zero when the memory card check found saved Freqs. */
    std::list<UnlockEvent> mUnlockScreens; /*!< The queued screens that follow the song. */
    std::vector<UnlockableItem> mUnlocks;  /*!< The items the last song unlocked. */
    Gizmo *mGizmo;                         /*!< The projector of the menu screens. +0x12c */
    Rnd::Cam *mUiCamera;                   /*!< The camera of the front end. */
    Rnd::Environ *mUiEnviron;              /*!< The lights of the front end. */
    int mSpeedingUp;                       /*!< Set while a `speed_up` transition runs. */
    float mSpeed;                          /*!< The rate of mTime against the scheduler. */
    int mNetScreenPending;                 /*!< Whether mNetScreen waits to be shown. */
    int mReserved144;  // +0x144, set to 1 by the constructor and not yet identified.
    String mNetScreen; /*!< The network screen to show once a launchpad screen shows. */

private:
    /**
     * Register every screen, panel, and component class of the front end with TheUI, build the
     * gizmo, and load the shared music.
     *
     * @ghidraAddress NTSC-U/C: 0x001634d8
     * @ghidraAddress PAL: 0x00166250
     * @stub
     */
    static void RegisterScreenClasses();

    /**
     * Build the gizmo.
     *
     * @ghidraAddress NTSC-U/C: 0x001644c8
     * @ghidraAddress PAL: 0x00167318
     */
    void CreateGizmo();

    /**
     * Start the memory card check that opens the start-up screens, start the front end clock, and
     * delete the loading screen.
     *
     * @ghidraAddress NTSC-U/C: 0x001650c0
     * @ghidraAddress PAL: 0x00167f80
     */
    void StartIntro();

    /**
     * Advance the loads of the front end by one stage of LoadStage when the current one is done.
     *
     * @ghidraAddress NTSC-U/C: 0x00165150
     * @ghidraAddress PAL: 0x00168010
     */
    void PollLoad();

    /**
     * Mark the front end loaded and, in kStateFrontEnd, move it to mNextScreen.
     *
     * @ghidraAddress NTSC-U/C: 0x001658c8
     * @ghidraAddress PAL: 0x00168838
     */
    void ShowFrontEnd();

    /**
     * Leave kStateFrontEnd for a song, choosing the screen to return to after it.
     *
     * @ghidraAddress NTSC-U/C: 0x00165930
     * @ghidraAddress PAL: 0x001688a0
     */
    void ExitFrontEnd();

    /**
     * Start the time the loading screen of a song shows for.
     *
     * @ghidraAddress NTSC-U/C: 0x00165c60
     * @ghidraAddress PAL: 0x00168c10
     */
    void StartLoading();

    /**
     * Leave kStateLoading. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00165d00
     * @ghidraAddress PAL: 0x00168cb0
     */
    void ExitLoading();

    /**
     * The part of EnterPlaying() that follows the change of state. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00165d08
     * @ghidraAddress PAL: 0x00168cb8
     */
    void StartPlaying();

    /**
     * Leave kStatePlaying. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00165d10
     * @ghidraAddress PAL: 0x00168cc0
     */
    void ExitPlaying();

    /**
     * Choose the screen to show after a remix song.
     *
     * @ghidraAddress NTSC-U/C: 0x00165d18
     * @ghidraAddress PAL: 0x00168cc8
     */
    void StartLeaving();

    /**
     * Leave kStateLeaving. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00165df0
     * @ghidraAddress PAL: 0x00168db8
     */
    void ExitLeaving();

    /**
     * Choose the next song of the campaign and start loading its sound banks again.
     *
     * @ghidraAddress NTSC-U/C: 0x00165df8
     * @ghidraAddress PAL: 0x00168dc0
     */
    void StartRestarting();

    /**
     * Leave kStateRestarting. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00165f10
     * @ghidraAddress PAL: 0x00168ed8
     */
    void ExitRestarting();

    /**
     * Leave the current state.
     *
     * @param nNextState The state about to be entered. The body does not read it.
     * @ghidraAddress NTSC-U/C: 0x00165f18
     * @ghidraAddress PAL: 0x00168ee0
     */
    void ExitState(int nNextState);

    /**
     * Record the result of a practice song in the profile of the first player.
     *
     * @ghidraAddress NTSC-U/C: 0x00166c18
     * @ghidraAddress PAL: 0x00169c08
     */
    void RecordPracticeResult();

    /**
     * Close the dialog over the song: report the choice once the front end shows `blank`, and
     * move it there otherwise.
     *
     * @ghidraAddress NTSC-U/C: 0x00166d80
     * @ghidraAddress PAL: 0x00169d70
     */
    void FinishDialog();

    /**
     * Report whether a move between two screens is listed in the `speed_up` entry of the
     * metagame configuration.
     *
     * @param pszFrom The screen left.
     * @param pszTo The screen entered.
     * @return Whether the move is listed.
     * @ghidraAddress NTSC-U/C: 0x00166fd0
     * @ghidraAddress PAL: 0x00169fc0
     */
    bool IsSpeedUp(const char *pszFrom, const char *pszTo);

    /**
     * Report whether a move between two screens is listed in the `stop_speed_up` entry of the
     * metagame configuration.
     *
     * @param pszFrom The screen left.
     * @param pszTo The screen entered.
     * @return Whether the move is listed.
     * @ghidraAddress NTSC-U/C: 0x001670d8
     * @ghidraAddress PAL: 0x0016a0c8
     */
    bool IsStopSpeedUp(const char *pszFrom, const char *pszTo);

    /**
     * Pass a button of a controller to the front end, outside a song.
     *
     * @param pMsg The message.
     * @return What UIManager::Dispatch() reports, true for a controller beyond the players, and
     * false for a button the metagame leaves to the song.
     * @ghidraAddress NTSC-U/C: 0x001671e0
     * @ghidraAddress PAL: 0x0016a1d0
     */
    bool OnJoypadInput(JoypadInputMsg *pMsg);

    /**
     * Refresh the help and act on the screen a move between screens reached.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001672f0
     */
    bool OnTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Fire the component select triggers for a choice made with Cross.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001675f8
     * @ghidraAddress PAL: 0x0016a688
     */
    bool OnComponentSelect(UIComponentSelectMsg *pMsg);

    /**
     * Fire the component select start triggers.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00167630
     * @ghidraAddress PAL: 0x0016a6c0
     */
    bool OnComponentSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Fire the focus triggers, and show the help of the component that took the focus.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00167658
     * @ghidraAddress PAL: 0x0016a6e8
     */
    bool OnComponentFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Fire the screen change triggers, show the title of the screen entered, and act on the
     * screen.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001676c8
     * @ghidraAddress PAL: 0x0016a758
     */
    bool OnScreenChange(UIScreenChangeMsg *pMsg);

    /**
     * Show the error of an aborted launchpad, at once in the front end or after the song.
     *
     * Nothing shows while the lobby error screen shows or is next.
     *
     * @param pMsg The message.
     * @return false.
     * @ghidraAddress NTSC-U/C: 0x00167b50
     * @ghidraAddress PAL: 0x0016abe0
     */
    bool OnLaunchpadAborted(LaunchpadAbortedMsg *pMsg);

    /**
     * Show the error of a lost connection to the lobby, at once in the front end or after the
     * song.
     *
     * @param pMsg The message.
     * @return false.
     * @ghidraAddress NTSC-U/C: 0x00167d78
     * @ghidraAddress PAL: 0x0016ae08
     */
    bool OnLostInternet(LobbyConnectionLostMsg *pMsg);

    /**
     * Take the settings of the online game the host published, keeping the read-only flag of the
     * remix in step, and show them on the launchpad screen when it is the current screen.
     *
     * @param pMsg The message.
     * @return false.
     * @ghidraAddress NTSC-U/C: 0x00167f50
     * @ghidraAddress PAL: 0x0016afe0
     */
    bool OnGameParamsUpdate(GameParamsUpdateMsg *pMsg);

    /**
     * Add the players of the launchpad to the game database, this console's player first, and go
     * to the launch sequence.
     *
     * A remix game without a loaded remix starts an untitled one. Every remix game records the
     * song and the names of the players as the creators. With no launchpad, the routine shows
     * that the session was lost.
     *
     * @param pMsg The message. It is not read.
     * @return false.
     * @ghidraAddress NTSC-U/C: 0x00168dc0
     * @ghidraAddress PAL: 0x0016bf48
     */
    bool OnLoadGame(Message *pMsg);

    /**
     * While the guest launchpad screens or the keyboard show, go to `net_share_remix`, or show
     * that the session was lost when there is no launchpad.
     *
     * @param pMsg The message. It is not read.
     * @return false.
     * @ghidraAddress NTSC-U/C: 0x001690c8
     * @ghidraAddress PAL: 0x0016c250
     */
    bool OnShareRemixBegin(Message *pMsg);

    /**
     * Pass the progress of a shared remix to `net_share_remix` while a launchpad screen,
     * `net_share_remix`, `net_launch`, or the keyboard is the current screen.
     *
     * @param pMsg The message.
     * @return false.
     * @ghidraAddress NTSC-U/C: 0x00169178
     * @ghidraAddress PAL: 0x0016c300
     */
    bool OnShareRemixProgress(ShareRemixProgressMsg *pMsg);

    /**
     * Divide the sample memory among the bank slots and start loading the transition effects
     * into the slot of the game effects.
     *
     * @ghidraAddress NTSC-U/C: 0x00168210
     */
    void LoadFxBanks();

    /**
     * Report whether the bank in the slot of the game effects has loaded.
     *
     * @return Whether the bank is loaded.
     * @ghidraAddress NTSC-U/C: 0x001685b8
     * @ghidraAddress PAL: 0x0016b740
     */
    bool IsGameFxBankLoaded();

    /**
     * Divide the sample memory again and start loading the effects of the front end, the shared
     * music, and the menu song.
     *
     * @ghidraAddress NTSC-U/C: 0x001685f0
     * @ghidraAddress PAL: 0x0016b778
     */
    void LoadMusicBanks();

    /**
     * Report whether every bank slot of the front end has loaded.
     *
     * @return Whether the banks are loaded.
     * @ghidraAddress NTSC-U/C: 0x001689b8
     * @ghidraAddress PAL: 0x0016bb40
     */
    bool AreBanksLoaded();

    /**
     * Empty the bank slots of the front end effects and music.
     *
     * @ghidraAddress NTSC-U/C: 0x00168a78
     * @ghidraAddress PAL: 0x0016bc00
     */
    void UnloadBanks();

    /**
     * Build the menu music and the menu song. The first visit plays the first song of the `songs`
     * entry, and every later visit a song at random.
     *
     * @ghidraAddress NTSC-U/C: 0x00168af8
     * @ghidraAddress PAL: 0x0016bc80
     */
    void CreateMusic();
};

/**
 * The metagame.
 *
 * @ghidraAddress NTSC-U/C: 0x00436740
 */
extern Metagame TheMetagame;

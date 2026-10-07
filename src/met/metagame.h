#pragma once

#include "app/msgsink.h"
#include "met/gizmo.h"
#include "msg/message.h"

/**
 * Front end and flow of the whole game, from the title screens through a song and back.
 *
 * The RTTI records the class as deriving from MsgSink, and the vptr sits at offset 0. The one
 * instance is TheMetagame.
 *
 * The main loop drives it together with WorldMgr. Update() reports what the front end requested,
 * and the main loop answers by moving WorldMgr and calling the matching Enter routine here.
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

    /** Kinds of dialog ShowDialog() shows over a song. */
    enum DialogType {
        kDialogSoloWon = 0,      /*!< The result of a won solo song, or of a practice song. */
        kDialogSoloLost = 1,     /*!< The result of a lost solo song. */
        kDialogEndGame = 2,      /*!< The results at the end of a multiplayer song. */
        kDialogPause = 5,        /*!< The pause menu. */
        kDialogNoController = 6, /*!< The pause menu of a disconnected controller. */
        kDialogTutorialEnd = 7,  /*!< The menu at the end of the tutorial. The name is inferred. */
    };

    /** Choices a dialog reports to its callback. */
    enum DialogAction {
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
     * Load one of the five loading screens at random from `metagame\loading%d.rnd` and draw it.
     *
     * The main loop calls the routine before the other subsystems start, and the screen therefore
     * shows while they load.
     *
     * @ghidraAddress NTSC-U/C: 0x00164500
     * @ghidraAddress PAL: 0x00167350
     */
    void ShowLoadingScreen();

    /**
     * Read the metagame configuration and register the screens and the script commands.
     *
     * @ghidraAddress NTSC-U/C: 0x00164650
     * @ghidraAddress PAL: 0x001674f0
     */
    void Init();

    /**
     * Unregister the script commands and release what Init() created.
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
     * Draw the part of the front end that goes under the world.
     *
     * @ghidraAddress NTSC-U/C: 0x001655e8
     * @ghidraAddress PAL: 0x001684f8
     */
    void Draw();

    /**
     * Draw the part of the front end that goes over the world.
     *
     * @ghidraAddress NTSC-U/C: 0x00165618
     * @ghidraAddress PAL: 0x00168528
     */
    void DrawOverlay();

    /**
     * Show the blank screen unless the word at `+0x40` is set.
     *
     * GameLogic::Start() schedules the routine shortly before the first bar. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00165048
     * @ghidraAddress PAL: 0x00167f08
     */
    void ShowBlankScreen();

    /**
     * Handle a message sent to the metagame.
     *
     * @param pMsg The message.
     * @return The result of the handler of the message's type, or false for any other message.
     * @ghidraAddress NTSC-U/C: 0x00168038
     * @ghidraAddress PAL: 0x0016b0c8
     */
    bool DispatchPriv(Message *pMsg) override;

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

    int mState;      /*!< One of State. */
    int mScreen;     /*!< Front end screen code. Draw() acts only on code 8. */
    int mReserved0C; // +0x0c, not yet identified.
    int mReserved10; // +0x10, set by EnterLeaving() and cleared by Update().
    int mEvent;      /*!< The Event Update() reports next. */
    // The members from +0x18 to +0x128 are not yet declared.
    Gizmo *mGizmo; /*!< The projector of the menu screens. +0x12c */
    // The members after +0x12c are not yet declared.
};

/**
 * The metagame.
 *
 * @ghidraAddress NTSC-U/C: 0x00436740
 */
extern Metagame TheMetagame;

#pragma once

#include "app/msgsink.h"
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
     * Handle a message sent to the metagame.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x00168038
     * @ghidraAddress PAL: 0x0016b0c8
     */
    void DispatchPriv(Message *pMsg) override;

    int mState;      /*!< One of State. */
    int mScreen;     /*!< Front end screen code. Draw() acts only on code 8. */
    int mReserved0C; // +0x0c, not yet identified.
    int mReserved10; // +0x10, set by EnterLeaving() and cleared by Update().
    int mEvent;      /*!< The Event Update() reports next. */
    // The members after +0x14 run past +0x12c and are not yet declared.
};

/**
 * The metagame.
 *
 * @ghidraAddress NTSC-U/C: 0x00436740
 */
extern Metagame TheMetagame;

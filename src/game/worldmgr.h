#pragma once

#include "game/inputmgr.h"
#include "game/world.h"

/**
 * Driver of the World that is loading, running, or being torn down.
 *
 * The class is not polymorphic and has no RTTI, and the name is inferred from the members it
 * manages. The one instance is the function-local static of shared(), and TheWorldMgr addresses it.
 *
 * Load() builds the World subclass for the rule set the game database selects, Start() attaches
 * an InputMgr, and Unload() destroys both. Poll() advances the state each frame and reports what
 * changed as an Event.
 *
 * Load(), Start(), Unload(), and Reset() each time their body on a stack Timer. The measurement is
 * never read.
 */
class WorldMgr {
public:
    /** Values of mState. */
    enum State {
        kStateIdle = 0,     /*!< No world exists. */
        kStateLoading = 1,  /*!< A world is loading its assets. */
        kStatePlaying = 2,  /*!< The world is running with input attached. */
        kStateUnloaded = 3, /*!< The world was destroyed and the display is changing back. */
    };

    /** Values Poll() reports. */
    enum Event {
        kEventNone = 0,     /*!< Nothing changed. */
        kEventLoaded = 1,   /*!< The loading world is ready to start. */
        kEventFinished = 2, /*!< The running world ended. */
        kEventUnloaded = 3, /*!< The display finished changing after Unload(). */
        kEventRestart = 4,  /*!< The running world ended and requested a restart. */
    };

    /**
     * Construct the driver with every member clear.
     *
     * @ghidraAddress NTSC-U/C: 0x00104878
     * @ghidraAddress PAL: 0x00105f60
     */
    WorldMgr();

    /**
     * Release the driver. The body is empty, and the world and the router are not destroyed.
     *
     * @ghidraAddress NTSC-U/C: 0x00104898
     * @ghidraAddress PAL: 0x00105f80
     */
    ~WorldMgr();

    /**
     * Report the single instance, constructing it on first use.
     *
     * @return The instance.
     * @ghidraAddress NTSC-U/C: 0x001049a0
     * @ghidraAddress PAL: 0x00106088
     */
    static WorldMgr *shared();

    /**
     * Initialise TheGfxManager and TheGameConfig, and mark the driver initialised.
     *
     * @ghidraAddress NTSC-U/C: 0x001048c0
     * @ghidraAddress PAL: 0x00105fa8
     */
    void Init();

    /**
     * Destroy the world and the router if either exists, and shut down TheGfxManager.
     *
     * The two pointers are not cleared.
     *
     * @ghidraAddress NTSC-U/C: 0x00104900
     * @ghidraAddress PAL: 0x00105fe8
     */
    void Terminate();

    /**
     * Report mState.
     *
     * @return One of State.
     * @ghidraAddress NTSC-U/C: 0x001049f8
     * @ghidraAddress PAL: 0x001060e0
     */
    int GetState() const;

    /**
     * Reset TheGfxManager and enter kStateIdle.
     *
     * @ghidraAddress NTSC-U/C: 0x00104a00
     * @ghidraAddress PAL: 0x001060e8
     */
    void Reset();

    /**
     * Build the World for the selected rule set and enter kStateLoading.
     *
     * Rule set 1 builds a Game, rule set 2 a Remix, and rule set 3 a Duel. Any other value builds
     * no world and passes "invalid ruleset" to DebugWarn(). The periodic controller check is
     * turned off until Unload().
     *
     * @ghidraAddress NTSC-U/C: 0x00104a70
     * @ghidraAddress PAL: 0x00106158
     */
    void Load();

    /**
     * Attach an InputMgr to the loaded world and enter kStatePlaying.
     *
     * @ghidraAddress NTSC-U/C: 0x00104bb8
     * @ghidraAddress PAL: 0x001062a0
     */
    void Start();

    /**
     * Destroy the router and the world and enter kStateUnloaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00104c50
     * @ghidraAddress PAL: 0x00106338
     */
    void Unload();

    /**
     * Copy two timing values of the running world into the game database.
     *
     * Only a world in kStatePlaying publishes. The main loop calls the routine first in each
     * frame. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00104d20
     * @ghidraAddress PAL: 0x00106408
     */
    void UpdateTime();

    /**
     * Advance the current state by one frame and report what changed.
     *
     * The pending event is cleared as it is reported.
     *
     * @return One of Event.
     * @ghidraAddress NTSC-U/C: 0x00104d88
     * @ghidraAddress PAL: 0x00106470
     */
    int Poll();

    /**
     * Draw TheGfxManager, at the running world's tick or at time 0 otherwise.
     *
     * @ghidraAddress NTSC-U/C: 0x00104ef0
     * @ghidraAddress PAL: 0x001065d8
     */
    void Draw();

private:
    /**
     * Send Reset All Controllers on every MIDI channel and restore the full output level of
     * TheSynth.
     *
     * Start() and Unload() call the routine. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001047d0
     * @ghidraAddress PAL: 0x00105eb8
     */
    static void ResetSynthControllers();

public:
    World *mWorld;       /*!< The world Load() built, or null. */
    InputMgr *mInputMgr; /*!< The router Start() attached, or null. */
    int mState;          /*!< One of State. */
    int mEvent;          /*!< The Event Poll() reports next. */
    int mInitialized;    /*!< Set by Init(). */
    int mWorldStarted;   /*!< Set once the loading world was told to load its assets. */
};

/**
 * The world driver, WorldMgr::shared() as the unit's static initialiser stored it.
 *
 * @ghidraAddress NTSC-U/C: 0x00435dcc
 */
extern WorldMgr *TheWorldMgr;

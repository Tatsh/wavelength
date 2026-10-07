#pragma once

#include "os/asyncstream.h"
#include "os/string.h"
#include "rnd/particlesysanim.h"
#include "rnd/rndloader.h"
#include "rnd/view.h"

/**
 * The arena scene shown behind the front end, and its arena unlock animations.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the
 * `Metagame/Arena/%s.rnd` file it loads, named after the `arena_prefix` entry of the metagame
 * configuration. Metagame builds the one instance. The scene loads first, then the triggers of the
 * `trigger_file` entry. An unlock reveal plays one `ArenaUnlockAnim_0N.view` and its particles.
 */
class MetagameArena {
public:
    /**
     * Build the arena with nothing loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00162808
     * @ghidraAddress PAL: 0x00165580
     */
    MetagameArena();

    /**
     * Delete the load of the arena scene.
     *
     * @ghidraAddress NTSC-U/C: 0x00162870
     * @ghidraAddress PAL: 0x001655e8
     */
    ~MetagameArena();

    /**
     * Start loading the arena scene across frames.
     *
     * @ghidraAddress NTSC-U/C: 0x001628d0
     * @ghidraAddress PAL: 0x00165648
     */
    void StartLoad();

    /**
     * Report whether the arena scene and its triggers have loaded.
     *
     * @return Whether the scene is ready.
     * @ghidraAddress NTSC-U/C: 0x00162988
     * @ghidraAddress PAL: 0x00165700
     */
    bool IsLoaded() const;

    /**
     * Set the frame of each unlock animation to show the arenas the player has unlocked.
     *
     * The animation of the arena being revealed stays at its first frame.
     *
     * @ghidraAddress NTSC-U/C: 0x00162cc8
     * @ghidraAddress PAL: 0x00165a40
     */
    void ShowUnlocks();

    /**
     * Stop the triggers and delete the arena scene.
     *
     * @ghidraAddress NTSC-U/C: 0x00162ef0
     * @ghidraAddress PAL: 0x00165c68
     */
    void Unload();

    /**
     * Advance the arena scene and the unlock animations by one frame.
     *
     * @param nState The metagame state, one of Metagame::State.
     * @param nLoadStage The metagame load stage, one of Metagame::LoadStage.
     * @param flTime The front end clock.
     * @param flTick The front end clock in ticks.
     * @ghidraAddress NTSC-U/C: 0x00162f38
     * @ghidraAddress PAL: 0x00165cb0
     */
    void Poll(int nState, int nLoadStage, float flTime, float flTick);

    /**
     * Draw the arena scene while the front end shows and the scene has loaded.
     *
     * @param nState The metagame state, one of Metagame::State.
     * @ghidraAddress NTSC-U/C: 0x00163160
     * @ghidraAddress PAL: 0x00165ed8
     */
    void Draw(int nState);

    /**
     * Choose the arena the next reveal shows.
     *
     * @param nArena The arena, counted from 1.
     * @ghidraAddress NTSC-U/C: 0x001631c8
     * @ghidraAddress PAL: 0x00165f40
     */
    void SetRevealArena(int nArena);

    /**
     * Start the animation that reveals the chosen arena.
     *
     * @param flTime The front end clock.
     * @ghidraAddress NTSC-U/C: 0x001631d8
     * @ghidraAddress PAL: 0x00165f50
     */
    void StartReveal(float flTime);

private:
    /** Values of mLoadState. */
    enum LoadState {
        kLoadStateIdle = 0,     /*!< Nothing is loaded. */
        kLoadStateScene = 1,    /*!< The scene loads. */
        kLoadStateTriggers = 2, /*!< The triggers load. */
        kLoadStateDone = 3,     /*!< The scene and the triggers have loaded. */
    };

    /**
     * Advance the load of the scene and of the triggers.
     *
     * @param flTime The front end clock. The body does not read it.
     * @ghidraAddress NTSC-U/C: 0x00162998
     * @ghidraAddress PAL: 0x00165710
     */
    void PollLoad(float flTime);

    RndLoader *mLoader;         /*!< The load of the scene, or null. */
    Rnd::View *mView;           /*!< The view of the arena. */
    Rnd::View *mTransitionView; /*!< The transition that drives the arena's sub-views. */
    String mPrefix;             /*!< The `arena_prefix` entry of the metagame configuration. */
    int mRevealArena;           /*!< The unlock animation to reveal, 0 for the fifth, or -1. */
    float mRevealStart;         /*!< The clock the reveal started at, or 0. */
    float mParticlesStart;      /*!< The clock the reveal's particles started at, or 0. */
    Rnd::View *mRevealView;     /*!< The animation of the reveal, or null. */
    Rnd::ParticleSysAnim *mParticles; /*!< The particles of the reveal, or null. */
    float mRevealLength;              /*!< The length of the reveal. */
    float mParticlesLength;           /*!< The length of the reveal's particles. */
    const char *mTriggerFile;    /*!< The `trigger_file` entry of the metagame configuration. */
    AsyncStream *mTriggerStream; /*!< The read of the trigger file, or null. */
    int mLoadState;              /*!< One of LoadState. */
};

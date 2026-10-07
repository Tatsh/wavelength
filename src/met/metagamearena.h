#pragma once

/**
 * The arena scene shown behind the front end, and its arena unlock animations.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the
 * `Metagame/Arena/%s.rnd` file it loads, named after the `arena_prefix` entry of the metagame
 * configuration. Metagame builds the one instance. Only the members the metagame uses are
 * declared.
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
     * Report whether the arena scene has loaded.
     *
     * @return Whether the scene is ready.
     * @ghidraAddress NTSC-U/C: 0x00162988
     * @ghidraAddress PAL: 0x00165700
     */
    bool IsLoaded() const;

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
     * Show the arenas the player has unlocked.
     *
     * @ghidraAddress NTSC-U/C: 0x00162cc8
     * @ghidraAddress PAL: 0x00165a40
     */
    void ShowUnlocks();

    /**
     * Start the animation that reveals the next arena.
     *
     * @param flTime The front end clock.
     * @ghidraAddress NTSC-U/C: 0x001631d8
     * @ghidraAddress PAL: 0x00165f50
     */
    void StartReveal(float flTime);
};

#pragma once

/**
 * The sound effects of the game, read from the `fx_midi_file` of the "db" section.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Each effect is a cue that
 * the one instance stores, and every member below plays or stops one cue of that instance. Only
 * the members GameLogic uses are declared.
 */
class GameFx {
public:
    /**
     * Play the sound of a cheat.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fba8
     * @ghidraAddress PAL: 0x002894a8
     */
    static void PlayCheat();

    /**
     * Stop the looping sound of a song.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fda0
     * @ghidraAddress PAL: 0x002896a0
     */
    static void StopLoop();

    /**
     * Play the sound of a deployed power-up.
     *
     * @param nPowerup The kind of power-up.
     * @ghidraAddress NTSC-U/C: 0x0027ff80
     * @ghidraAddress PAL: 0x00289880
     */
    static void PlayPowerup(int nPowerup);
};

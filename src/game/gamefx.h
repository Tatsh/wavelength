#pragma once

/**
 * The sound effects of the game, read from the `fx_midi_file` of the "db" section.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Each effect is a cue that
 * the one instance stores, and every member below plays or stops one cue of that instance. Only
 * the members GameLogic and the front end use are declared.
 */
class GameFx {
public:
    /**
     * Play the sound of the left directional button in the menus.
     *
     * @ghidraAddress NTSC-U/C: 0x0027f9f8
     * @ghidraAddress PAL: 0x002892f8
     */
    static void PlayMenuLeft();

    /**
     * Play the sound of the right directional button in the menus.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fa40
     * @ghidraAddress PAL: 0x00289340
     */
    static void PlayMenuRight();

    /**
     * Play the sound of the up directional button in the menus.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fa88
     * @ghidraAddress PAL: 0x00289388
     */
    static void PlayMenuUp();

    /**
     * Play the sound of the down directional button in the menus.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fad0
     * @ghidraAddress PAL: 0x002893d0
     */
    static void PlayMenuDown();

    /**
     * Play the sound of a choice in the menus.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fb18
     * @ghidraAddress PAL: 0x00289418
     */
    static void PlayMenuSelect();

    /**
     * Play the sound of the projector that moves between the menu screens.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fbf0
     * @ghidraAddress PAL: 0x002894f0
     */
    static void PlayProjector();

    /**
     * Start the sound of a screen that asks for a transition sound.
     *
     * @ghidraAddress NTSC-U/C: 0x00280928
     * @ghidraAddress PAL: 0x0028a228
     */
    static void PlayTransition();

    /**
     * Stop the sound PlayTransition() started.
     *
     * @ghidraAddress NTSC-U/C: 0x00280970
     * @ghidraAddress PAL: 0x0028a270
     */
    static void StopTransition();

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

#pragma once

/**
 * Bank of interface sounds built from the "fx_midi_file" entry of the "db" configuration section.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. One instance exists, at the
 * pointer `0x00440d58`, and its members are one sound cue each. Every member below plays or stops
 * one cue of that instance. Only the members its callers here use are declared, and their names are
 * inferred from the events that play them.
 */
class FxMidi {
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
     * Play the sound of going back a screen in the front end.
     *
     * The metagame plays it when Triangle led away from a screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fb60
     * @ghidraAddress PAL: 0x00289460
     */
    static void PlayBack();

    /**
     * Play the sound of a cheat.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fba8
     * @ghidraAddress PAL: 0x002894a8
     */
    static void PlayCheat();

    /**
     * Play the sound of the projector that moves between the menu screens.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fbf0
     * @ghidraAddress PAL: 0x002894f0
     */
    static void PlayProjector();

    /**
     * Play the sound of the first handle.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fc38
     * @ghidraAddress PAL: 0x00289538
     */
    static void PlaySound0();

    /**
     * Play the sound of the second handle.
     *
     * A pitch track plays it when a player edits a bar another player owns.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fc80
     * @ghidraAddress PAL: 0x00289580
     */
    static void PlaySound1();

    /**
     * Play the sound of the third handle.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fcc8
     * @ghidraAddress PAL: 0x002895c8
     */
    static void PlaySound2();

    /**
     * Play the sound of the fourth handle.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fd10
     * @ghidraAddress PAL: 0x00289610
     */
    static void PlaySound3();

    /**
     * Play the sound of a won song.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fd58
     * @ghidraAddress PAL: 0x00289658
     */
    static void PlayWinSound();

    /**
     * Stop the looping sound of a song.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fda0
     * @ghidraAddress PAL: 0x002896a0
     */
    static void StopLoop();

    /**
     * Play the sound that warns that the juice of a solo player is low.
     *
     * @ghidraAddress NTSC-U/C: 0x0027fea0
     * @ghidraAddress PAL: 0x002897a0
     */
    static void PlayJuiceLowSound();

    /**
     * Play the sound of a power-up a player caught.
     *
     * @param nPowerup The kind of power-up, one of GameLogic::Powerup.
     * @ghidraAddress NTSC-U/C: 0x0027ff30
     * @ghidraAddress PAL: 0x00289830
     */
    static void PlayPowerupCatchSound(int nPowerup);

    /**
     * Play the sound of a deployed power-up.
     *
     * @param nPowerup The kind of power-up, one of GameLogic::Powerup.
     * @ghidraAddress NTSC-U/C: 0x0027ff80
     * @ghidraAddress PAL: 0x00289880
     */
    static void PlayPowerupSound(int nPowerup);

    /**
     * Play the guide sound of a gem lane.
     *
     * GuideTicker plays it for each gem of an enabled bar.
     *
     * @param nLane The lane.
     * @ghidraAddress NTSC-U/C: 0x0027ffd0
     * @ghidraAddress PAL: 0x002898d0
     */
    static void PlayGuideSound(int nLane);

    /**
     * Report whether a leader sound is playing.
     *
     * @return Whether one of the four leader sounds plays.
     * @ghidraAddress NTSC-U/C: 0x00280020
     * @ghidraAddress PAL: 0x00289920
     */
    static bool IsLeaderSoundPlaying();

    /**
     * Play the sound that announces a new leader.
     *
     * @param nSlot The leader's player slot.
     * @ghidraAddress NTSC-U/C: 0x00280098
     * @ghidraAddress PAL: 0x00289998
     */
    static void PlayLeaderSound(int nSlot);

    /**
     * Play the sound that announces the winner.
     *
     * @param nSlot The winner's player slot.
     * @ghidraAddress NTSC-U/C: 0x002800e8
     * @ghidraAddress PAL: 0x002899e8
     */
    static void PlayWinnerSound(int nSlot);

    /**
     * Play `DUEL_LAYPATT`.
     *
     * @ghidraAddress NTSC-U/C: 0x00280140
     * @ghidraAddress PAL: 0x00289a40
     */
    static void PlayDuelLayPattern();

    /**
     * Play `DUEL_CATCHPATT`.
     *
     * @ghidraAddress NTSC-U/C: 0x00280188
     * @ghidraAddress PAL: 0x00289a88
     */
    static void PlayDuelCatchPattern();

    /**
     * Play `DUEL_NICE`.
     *
     * @ghidraAddress NTSC-U/C: 0x002801d0
     * @ghidraAddress PAL: 0x00289ad0
     */
    static void PlayDuelNice();

    /**
     * Play `DUEL_YOUGOTIT`.
     *
     * @ghidraAddress NTSC-U/C: 0x00280218
     * @ghidraAddress PAL: 0x00289b18
     */
    static void PlayDuelYouGotIt();

    /**
     * Play `DUEL_ALMOST`.
     *
     * @ghidraAddress NTSC-U/C: 0x00280260
     * @ghidraAddress PAL: 0x00289b60
     */
    static void PlayDuelAlmost();

    /**
     * Play `DUEL_ONELETTER`.
     *
     * @ghidraAddress NTSC-U/C: 0x002802a8
     * @ghidraAddress PAL: 0x00289ba8
     */
    static void PlayDuelOneLetter();

    /**
     * Play `DUEL_AWW`.
     *
     * @ghidraAddress NTSC-U/C: 0x002802f0
     * @ghidraAddress PAL: 0x00289bf0
     */
    static void PlayDuelAww();

    /**
     * Play `DUEL_PERFECT`.
     *
     * @ghidraAddress NTSC-U/C: 0x00280338
     * @ghidraAddress PAL: 0x00289c38
     */
    static void PlayDuelPerfect();

    /**
     * Play `DUEL_GAMETIE`.
     *
     * @ghidraAddress NTSC-U/C: 0x00280388
     * @ghidraAddress PAL: 0x00289c88
     */
    static void PlayDuelGameTie();

    /**
     * Play `DUEL_GREENWINS`.
     *
     * @ghidraAddress NTSC-U/C: 0x002803d0
     * @ghidraAddress PAL: 0x00289cd0
     */
    static void PlayDuelGreenWins();

    /**
     * Play `DUEL_PURPLEWINS`.
     *
     * @ghidraAddress NTSC-U/C: 0x00280418
     * @ghidraAddress PAL: 0x00289d18
     */
    static void PlayDuelPurpleWins();

    /**
     * Play `DUEL_MISS`.
     *
     * @ghidraAddress NTSC-U/C: 0x00280460
     * @ghidraAddress PAL: 0x00289d60
     */
    static void PlayDuelMiss();

    /**
     * Play `DUEL_MISSTHIS`.
     *
     * @ghidraAddress NTSC-U/C: 0x002804a8
     * @ghidraAddress PAL: 0x00289da8
     */
    static void PlayDuelMissThis();

    /**
     * Play `DUEL_CHEER`.
     *
     * @ghidraAddress NTSC-U/C: 0x002804f0
     * @ghidraAddress PAL: 0x00289df0
     */
    static void PlayDuelCheer();

    /**
     * Play the sound of a gem or bar erased in the remix editor.
     *
     * @ghidraAddress NTSC-U/C: 0x00280538
     * @ghidraAddress PAL: 0x00289e38
     */
    static void PlayEraseSound();

    /**
     * Play the sound of a section erased in the remix editor.
     *
     * @ghidraAddress NTSC-U/C: 0x00280580
     * @ghidraAddress PAL: 0x00289e80
     */
    static void PlayEraseSectionSound();

    /**
     * Play the `WRONG` cue of a choice that is not allowed.
     *
     * @ghidraAddress NTSC-U/C: 0x002805c8
     * @ghidraAddress PAL: 0x00289ec8
     */
    static void PlayWrong();

    /**
     * Play the `SQUARE` cue. The song screen plays it to start a practice song.
     *
     * @ghidraAddress NTSC-U/C: 0x00280610
     * @ghidraAddress PAL: 0x00289f10
     */
    static void PlaySquare();

    /**
     * Play the `LAZYSUSAN` cue. The arena screen plays it when the focus moves to another arena.
     *
     * @ghidraAddress NTSC-U/C: 0x00280658
     * @ghidraAddress PAL: 0x00289f58
     */
    static void PlayLazySusan();

    /**
     * Play the `KEYBOARD_LEFT_UP` cue of the front-end keyboard.
     *
     * @ghidraAddress NTSC-U/C: 0x002806a0
     * @ghidraAddress PAL: 0x00289fa0
     */
    static void PlayKeyboardLeftUp();

    /**
     * Play the `KEYBOARD_BACK` cue of the front-end keyboard.
     *
     * @ghidraAddress NTSC-U/C: 0x002806f8
     * @ghidraAddress PAL: 0x00289ff8
     */
    static void PlayKeyboardBack();

    /**
     * Play the `KEYBOARD_KEYENTER` cue of the front-end keyboard.
     *
     * @ghidraAddress NTSC-U/C: 0x00280740
     * @ghidraAddress PAL: 0x0028a040
     */
    static void PlayKeyboardKeyEnter();

    /**
     * Play the `ARENA_UNLOCK` cue of an arena that has just been unlocked.
     *
     * @ghidraAddress NTSC-U/C: 0x00280788
     * @ghidraAddress PAL: 0x0028a088
     */
    static void PlayArenaUnlock();

    /**
     * Play the `SOLOPORTAL` cue of the solo button of the main menu.
     *
     * @ghidraAddress NTSC-U/C: 0x002807d0
     * @ghidraAddress PAL: 0x0028a0d0
     */
    static void PlaySoloPortal();

    /**
     * Play the `MULTIPORTAL` cue of the multiplayer button of the main menu.
     *
     * @ghidraAddress NTSC-U/C: 0x00280818
     * @ghidraAddress PAL: 0x0028a118
     */
    static void PlayMultiPortal();

    /**
     * Play the `NETPORTAL` cue of the online button of the main menu.
     *
     * @ghidraAddress NTSC-U/C: 0x00280860
     * @ghidraAddress PAL: 0x0028a160
     */
    static void PlayNetPortal();

    /**
     * Stop the three portal cues of the main menu.
     *
     * @ghidraAddress NTSC-U/C: 0x002808a8
     * @ghidraAddress PAL: 0x0028a1a8
     */
    static void StopPortals();

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
};

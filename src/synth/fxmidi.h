#pragma once

/**
 * Bank of interface sounds built from the "fx_midi_file" entry of the "db" configuration section.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. One instance exists, at the
 * pointer `0x00440d58`, and its members are one sound handle each. Only the members its callers
 * here use are declared, and their names are inferred from the events that play them.
 */
class FxMidi {
public:
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
     * Play the sound of a power-up.
     *
     * @param nPowerup The kind of power-up, one of GameLogic::Powerup.
     * @ghidraAddress NTSC-U/C: 0x0027ff80
     * @ghidraAddress PAL: 0x00289880
     */
    static void PlayPowerupSound(int nPowerup);

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
};

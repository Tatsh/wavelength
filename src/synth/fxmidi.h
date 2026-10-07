#pragma once

/**
 * Bank of interface sounds built from the "fx_midi_file" entry of the "db" configuration section.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. One instance exists, at the
 * pointer `0x00440d58`, and its members are one sound handle each. This header declares only the
 * members the tutorials and the solo game logic call.
 */
class FxMidi {
public:
    /**
     * Play the sound of the second handle.
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
};

#pragma once

#include "mid/midireceiver.h"

/**
 * Reader of a Standard MIDI File that reports each track and event to a MidiReceiver.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Only the members the front
 * end uses are declared, and the routines are not reconstructed.
 */
class MidiReader {
public:
    /**
     * Open a MIDI file.
     *
     * @param pszFile The file.
     * @param pReceiver The receiver of the tracks and events.
     * @ghidraAddress NTSC-U/C: 0x00158fa8
     * @ghidraAddress PAL: 0x0015a830
     */
    MidiReader(const char *pszFile, MidiReceiver *pReceiver);

    /**
     * Close the file.
     *
     * @ghidraAddress NTSC-U/C: 0x001590f0
     * @ghidraAddress PAL: 0x0015a978
     */
    ~MidiReader();

    /**
     * Read the whole file.
     *
     * @ghidraAddress NTSC-U/C: 0x00159198
     * @ghidraAddress PAL: 0x0015aa20
     */
    void ReadAll();
};

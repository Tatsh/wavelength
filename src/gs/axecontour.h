#pragma once

#include "gs/muse.h"

/**
 * The notes one gem button of a guitar track plays over one span of the song.
 *
 * The RTTI includes the class name. Only the members the guitar track uses are declared.
 */
class AxeContour {
public:
    /**
     * Construct empty notes.
     *
     * @param nLength The length of the notes in ticks.
     * @param nChannel The MIDI channel of the notes.
     * @ghidraAddress NTSC-U/C: 0x00156bf0
     * @ghidraAddress PAL: 0x00158478
     */
    AxeContour(int nLength, unsigned char nChannel);

    /**
     * Release the notes.
     *
     * @ghidraAddress NTSC-U/C: 0x00156cc0
     * @ghidraAddress PAL: 0x00158548
     */
    ~AxeContour();

    /**
     * Add a note.
     *
     * @param nTick The tick of the note within the notes.
     * @param nNote The note.
     * @param nVelocity The velocity.
     * @param nDuration The duration in ticks.
     * @ghidraAddress NTSC-U/C: 0x00156dc8
     * @ghidraAddress PAL: 0x00158650
     */
    void AddNote(int nTick, unsigned char nNote, unsigned char nVelocity, int nDuration);

    /**
     * Add a muse that plays with the notes.
     *
     * @param nTick The tick of the muse within the notes.
     * @param pMuse The muse, which the notes then manage.
     * @ghidraAddress NTSC-U/C: 0x00157078
     * @ghidraAddress PAL: 0x00158900
     */
    void Add(int nTick, Muse *pMuse);
};

#pragma once

#include "gs/axecontour.h"
#include "gs/axeharmony.h"

/**
 * Player of the notes one gem button of a guitar track plays.
 *
 * The RTTI includes the class name. The object is 0x20 bytes. Only the members AxeTrack uses are
 * declared, and the member names are inferred.
 */
class Axer {
public:
    /**
     * Construct a player.
     *
     * @param pContour The notes of the gem button.
     * @param pHarmony The chord the notes are taken from.
     * @param nReserved The value AxeTrackData records second. Its use is not yet identified.
     * @param fVolume The volume of the sound effects.
     * @ghidraAddress NTSC-U/C: 0x00157628
     * @ghidraAddress PAL: 0x00158eb0
     */
    Axer(AxeContour *pContour, const AxeHarmony *pHarmony, int nReserved, float fVolume);

    /**
     * Release the player.
     *
     * @ghidraAddress NTSC-U/C: 0x00157678
     * @ghidraAddress PAL: 0x00158f00
     */
    ~Axer();

    /**
     * Replace the notes.
     *
     * @param pContour The notes.
     * @ghidraAddress NTSC-U/C: 0x001576c0
     * @ghidraAddress PAL: 0x00158f48
     */
    void SetContour(AxeContour *pContour);

    /**
     * Replace the chord.
     *
     * @param pHarmony The chord.
     * @ghidraAddress NTSC-U/C: 0x00157738
     * @ghidraAddress PAL: 0x00158fc0
     */
    void SetHarmony(const AxeHarmony *pHarmony);

    /**
     * Report the length of the notes.
     *
     * @return The length in ticks.
     * @ghidraAddress NTSC-U/C: 0x00157758
     * @ghidraAddress PAL: 0x00158fe0
     */
    int GetLength();

    /**
     * Start the notes, unless they play.
     *
     * @param nOffset The tick within the notes to start at.
     * @ghidraAddress NTSC-U/C: 0x00157778
     * @ghidraAddress PAL: 0x00159000
     */
    void Start(int nOffset);

    /**
     * Stop the notes, if they play.
     *
     * @ghidraAddress NTSC-U/C: 0x00157800
     * @ghidraAddress PAL: 0x00159088
     */
    void Stop();

    /**
     * Report whether the notes play.
     *
     * @return Nonzero while the notes play.
     * @ghidraAddress NTSC-U/C: 0x00157858
     * @ghidraAddress PAL: 0x001590e0
     */
    int IsPlaying();

    /**
     * Set the horizontal stick position, which selects the note.
     *
     * @param fX The position.
     * @ghidraAddress NTSC-U/C: 0x00157860
     * @ghidraAddress PAL: 0x001590e8
     */
    void SetPosition(float fX);

    /**
     * Set the vertical stick position, which bends the pitch.
     *
     * @param fY The position.
     * @ghidraAddress NTSC-U/C: 0x00157940
     * @ghidraAddress PAL: 0x001591c8
     */
    void SetPitch(float fY);
};

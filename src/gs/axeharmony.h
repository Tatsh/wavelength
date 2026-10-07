#pragma once

#include <set>

#include "gs/axecontour.h"

/**
 * The notes of one harmony of a guitar track, from one song position onward.
 *
 * The class is not polymorphic. The RTTI of AxeTrackData's store of harmonies includes the class
 * name. The notes form a set, and the harmony records the middle of its range.
 */
class AxeHarmony {
public:
    /**
     * Construct a harmony with no notes.
     *
     * @ghidraAddress NTSC-U/C: 0x001571a8
     * @ghidraAddress PAL: 0x00158a30
     */
    AxeHarmony();

    /**
     * Release the notes.
     *
     * @ghidraAddress NTSC-U/C: 0x00157240
     * @ghidraAddress PAL: 0x00158ac8
     */
    ~AxeHarmony();

    /**
     * Add a note and move the middle of the range to fit it.
     *
     * @param nNote The note.
     * @ghidraAddress NTSC-U/C: 0x001572c0
     * @ghidraAddress PAL: 0x00158b48
     */
    void AddNote(unsigned char nNote);

    /**
     * Report the highest note.
     *
     * @return The note.
     * @ghidraAddress NTSC-U/C: 0x00157340
     * @ghidraAddress PAL: 0x00158bc8
     */
    unsigned char GetHighest() const;

    /**
     * Report the lowest note.
     *
     * @return The note.
     * @ghidraAddress NTSC-U/C: 0x00157368
     * @ghidraAddress PAL: 0x00158bf0
     */
    unsigned char GetLowest() const;

    /**
     * Move the notes of a contour onto the harmony, centred on its range.
     *
     * The first note snaps to the nearest note of the harmony. Each later note moves by the step
     * the contour takes from the note before it and snaps again, stepping on to the next note of
     * the harmony in the direction of the step when the snap would repeat the previous note.
     *
     * @param pContour The contour.
     * @param nTranspose The notes the contour moves by before it snaps.
     * @ghidraAddress NTSC-U/C: 0x00157378
     * @ghidraAddress PAL: 0x00158c00
     */
    void SnapContour(AxeContour *pContour, int nTranspose) const;

    /**
     * Report the note of the harmony nearest a note.
     *
     * A note below 0 counts as 0. A note midway between two notes of the harmony resolves to the
     * higher one.
     *
     * @param nNote The note.
     * @return The nearest note.
     * @ghidraAddress NTSC-U/C: 0x00157568
     * @ghidraAddress PAL: 0x00158df0
     */
    unsigned char Snap(int nNote) const;

private:
    std::set<unsigned char> mNotes; /*!< The notes. */
    float mCenter;                  /*!< The middle of the range of the notes. */
};

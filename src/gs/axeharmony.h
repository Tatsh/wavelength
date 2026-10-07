#pragma once

/**
 * The notes of the chord a guitar track plays over one span of the song.
 *
 * The RTTI includes the class name. Only the members the guitar track uses are declared.
 */
class AxeHarmony {
public:
    /**
     * Construct an empty chord.
     *
     * @ghidraAddress NTSC-U/C: 0x001571a8
     * @ghidraAddress PAL: 0x00158a30
     */
    AxeHarmony();

    /**
     * Release the chord.
     *
     * @ghidraAddress NTSC-U/C: 0x00157240
     * @ghidraAddress PAL: 0x00158ac8
     */
    ~AxeHarmony();

    /**
     * Add a note to the chord.
     *
     * @param nNote The note.
     * @ghidraAddress NTSC-U/C: 0x001572c0
     * @ghidraAddress PAL: 0x00158b48
     */
    void AddNote(unsigned char nNote);
};

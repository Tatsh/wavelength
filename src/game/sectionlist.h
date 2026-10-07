#pragma once

#include <vector>

/**
 * The sections of a song, as the bar each section ends at.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The first section starts at
 * bar 0, and every other section starts where the previous one ends.
 */
class SectionList {
public:
    /**
     * Report the number of sections.
     *
     * @return The number of sections.
     * @ghidraAddress NTSC-U/C: 0x00137430
     * @ghidraAddress PAL: 0x00138c90
     */
    int GetNumSections() const;

    /**
     * Report whether a bar is at or after the end of the last section.
     *
     * @param nBar The bar.
     * @return Whether the bar is past the last section.
     * @ghidraAddress NTSC-U/C: 0x00137448
     * @ghidraAddress PAL: 0x00138ca8
     */
    bool IsPastEnd(int nBar) const;

    /**
     * Report the bar a section starts at.
     *
     * @param nSection The section.
     * @return The end of the previous section, or 0 for the first section.
     * @ghidraAddress NTSC-U/C: 0x00137460
     * @ghidraAddress PAL: 0x00138cc0
     */
    int GetSectionStart(int nSection) const;

    /**
     * Report the bar a section ends at.
     *
     * @param nSection The section.
     * @return The bar after the last bar of the section.
     * @ghidraAddress NTSC-U/C: 0x00137490
     * @ghidraAddress PAL: 0x00138cf0
     */
    int GetSectionEnd(int nSection) const;

    std::vector<int> mEnds; /*!< The bar each section ends at. */
};

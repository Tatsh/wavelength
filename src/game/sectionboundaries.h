#pragma once

#include <vector>

/**
 * Ascending list of the measures at which the sections of a song end.
 *
 * Section 0 starts at measure 0, and every later section starts where the one before it ends. The
 * class is not polymorphic and has no RTTI. The name is inferred.
 */
class SectionBoundaries {
public:
    /**
     * Report the number of sections.
     *
     * @return The number of sections.
     * @ghidraAddress NTSC-U/C: 0x00137430
     * @ghidraAddress PAL: 0x00138c90
     */
    int NumSections() const;

    /**
     * Report whether a measure lies at or after the end of the last section.
     *
     * @param nMeasure The measure.
     * @return True when the measure is past the last section.
     * @ghidraAddress NTSC-U/C: 0x00137448
     * @ghidraAddress PAL: 0x00138ca8
     */
    bool IsPastEnd(int nMeasure) const;

    /**
     * Report the measure at which a section starts.
     *
     * @param nSection The section.
     * @return The first measure of the section.
     * @ghidraAddress NTSC-U/C: 0x00137460
     * @ghidraAddress PAL: 0x00138cc0
     */
    int SectionStart(int nSection) const;

    /**
     * Report the measure at which a section ends.
     *
     * @param nSection The section.
     * @return The first measure after the section.
     * @ghidraAddress NTSC-U/C: 0x00137490
     * @ghidraAddress PAL: 0x00138cf0
     */
    int SectionEnd(int nSection) const;

    /**
     * Report the measures a section spans.
     *
     * @param nSection The section.
     * @param pStart Receives the first measure of the section.
     * @param pEnd Receives the first measure after the section.
     * @ghidraAddress NTSC-U/C: 0x001374a8
     * @ghidraAddress PAL: 0x00138d08
     */
    void GetSectionRange(int nSection, int *pStart, int *pEnd) const;

    /**
     * Report the section a measure lies in.
     *
     * @param nMeasure The measure.
     * @return The section, which equals NumSections() past the last section.
     * @ghidraAddress NTSC-U/C: 0x001374f0
     * @ghidraAddress PAL: 0x00138d50
     */
    int SectionAt(int nMeasure) const;

    std::vector<int> mEnds; /*!< The end measure of each section, ascending. */
};

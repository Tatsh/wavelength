#pragma once

#include <vector>

/**
 * The pattern each section of a remixed song repeats, as the section the pattern comes from.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred.
 */
class SlotGrid {
public:
    /**
     * Construct a grid in which no section repeats a pattern.
     *
     * @param nSections The number of sections.
     * @ghidraAddress NTSC-U/C: 0x0012fc30
     * @ghidraAddress PAL: 0x00131458
     */
    explicit SlotGrid(int nSections);

    /**
     * Set the pattern a section repeats, and count the sections that repeat none.
     *
     * @param nPattern The section the pattern comes from, or -1.
     * @param nSection The section.
     * @ghidraAddress NTSC-U/C: 0x0012fcf0
     * @ghidraAddress PAL: 0x00131518
     */
    void SetPattern(int nPattern, int nSection);

    /**
     * Report whether a section repeats a pattern.
     *
     * @param nSection The section.
     * @return Whether the section's pattern is not -1.
     * @ghidraAddress NTSC-U/C: 0x0012fd50
     * @ghidraAddress PAL: 0x00131578
     */
    bool HasPattern(int nSection) const;

    /**
     * Report the pattern a section repeats.
     *
     * @param nSection The section.
     * @return The section the pattern comes from, or -1.
     * @ghidraAddress NTSC-U/C: 0x0012fd70
     * @ghidraAddress PAL: 0x00131598
     */
    int GetPattern(int nSection) const;

    /**
     * Report the number of sections.
     *
     * @return The number of sections.
     * @ghidraAddress NTSC-U/C: 0x0012fd88
     * @ghidraAddress PAL: 0x001315b0
     */
    int GetNumSections() const;

    /**
     * Report the number of sections that repeat no pattern.
     *
     * @return mNumFree.
     * @ghidraAddress NTSC-U/C: 0x0012fda0
     * @ghidraAddress PAL: 0x001315c8
     */
    int GetNumFree() const;

private:
    std::vector<int> mPatterns; /*!< The pattern of each section, or -1. */
    int mNumFree;               /*!< The number of sections that repeat no pattern. */
};

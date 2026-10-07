#pragma once

#include <vector>

#include "game/pitchgem.h"
#include "game/sectionboundaries.h"
#include "game/slotgrid.h"
#include "os/binstream.h"

/**
 * The gems of a pitch track, by the bar of the song as written they lie in.
 *
 * The class is not polymorphic. The name comes from the RTTI of its nested class SecStats. Each bar
 * keeps its gems in tick order. The owner of a section is the player who placed its gems, and a
 * section whose gems were all removed has no owner.
 */
class PitchTrackGems {
public:
    /** The owner of a section, and the number of gems that record the owner. */
    struct SecStats {
        int mOwner;   /*!< The player who owns the section, or -1. */
        int mNumGems; /*!< The gems of the section placed by the owner. */
    };

    /**
     * Construct a track of bars with no gems.
     *
     * @param pSections The sections of the song.
     * @param pPatterns The pattern each section repeats, or null.
     * @param nNumBars The number of bars.
     * @param nTicksPerBar The song ticks in one bar.
     * @ghidraAddress NTSC-U/C: 0x0012aaa8
     * @ghidraAddress PAL: 0x0012c2a0
     */
    PitchTrackGems(SectionBoundaries *pSections,
                   SlotGrid *pPatterns,
                   int nNumBars,
                   int nTicksPerBar);

    /**
     * Release the gems.
     *
     * @ghidraAddress NTSC-U/C: 0x0012ad50
     * @ghidraAddress PAL: 0x0012c548
     */
    ~PitchTrackGems();

    /**
     * Place a gem, replacing a gem at the same tick.
     *
     * @param nSlot The gem button.
     * @param nTick The tick.
     * @param nOwner The player who places the gem, or -1.
     * @return Whether a gem at the same tick was replaced.
     * @ghidraAddress NTSC-U/C: 0x0012aeb8
     * @ghidraAddress PAL: 0x0012c6b0
     */
    bool AddGem(signed char nSlot, int nTick, signed char nOwner);

    /**
     * Remove the gem at a tick.
     *
     * @param nTick The tick.
     * @return Whether a gem was removed.
     * @ghidraAddress NTSC-U/C: 0x0012b278
     * @ghidraAddress PAL: 0x0012ca70
     */
    bool RemoveGem(int nTick);

    /**
     * Remove the gem at a tick when it lies under the same gem button, and place it otherwise.
     *
     * @param nSlot The gem button.
     * @param nTick The tick.
     * @param nOwner The player who places the gem, or -1.
     * @return Whether a gem lies at the tick afterwards.
     * @ghidraAddress NTSC-U/C: 0x0012b3a0
     * @ghidraAddress PAL: 0x0012cb98
     */
    bool ToggleGem(signed char nSlot, int nTick, signed char nOwner);

    /**
     * Report the owner of the section a bar lies in.
     *
     * @param nBar The bar.
     * @return The player, or -1.
     * @ghidraAddress NTSC-U/C: 0x0012b7e8
     * @ghidraAddress PAL: 0x0012cfe0
     */
    int GetOwner(int nBar);

    /**
     * Remove every gem of a bar.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x0012b820
     * @ghidraAddress PAL: 0x0012d018
     */
    void ClearBar(int nBar);

    /**
     * Remove every gem of a section and its owner.
     *
     * @param nSection The section.
     * @ghidraAddress NTSC-U/C: 0x0012b920
     * @ghidraAddress PAL: 0x0012d118
     */
    void ClearSection(int nSection);

    /**
     * Copy the gems of a range of bars to other bars.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @param nDestBar The bar the copy of the first bar goes to.
     * @ghidraAddress NTSC-U/C: 0x0012b9c8
     * @ghidraAddress PAL: 0x0012d1c0
     */
    void CopyBars(int nStartBar, int nEndBar, int nDestBar);

    /**
     * Copy the gems of a range of bars of other gems to the same bars.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @param pSource The gems copied.
     * @param nOwner The owner the copies receive, or -1 to retain the owner of each gem.
     * @ghidraAddress NTSC-U/C: 0x0012b9e8
     * @ghidraAddress PAL: 0x0012d1e0
     */
    void CopyBarsFrom(int nStartBar, int nEndBar, PitchTrackGems *pSource, int nOwner);

    /**
     * Copy the gems of a range of bars of other gems, and record the owners of the sections the
     * copies land in.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @param nDestBar The bar the copy of the first bar goes to.
     * @param pSource The gems copied.
     * @param nOwner The owner the copies receive, or -1 to retain the owner of each gem.
     * @ghidraAddress NTSC-U/C: 0x0012ba10
     * @ghidraAddress PAL: 0x0012d208
     */
    void CopyBars(int nStartBar, int nEndBar, int nDestBar, PitchTrackGems *pSource, int nOwner);

    /**
     * Copy the gems of each section's pattern into the section.
     *
     * @ghidraAddress NTSC-U/C: 0x0012bd38
     * @ghidraAddress PAL: 0x0012d530
     */
    void ApplyPatterns();

    /**
     * Report the gems of a bar.
     *
     * @param nBar The bar.
     * @return The gems.
     * @ghidraAddress NTSC-U/C: 0x0012bdf0
     * @ghidraAddress PAL: 0x0012d5e8
     */
    std::vector<PitchGem> *GetBar(int nBar);

    /**
     * Count the gems of a range of bars.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @return The number of gems.
     * @ghidraAddress NTSC-U/C: 0x0012be08
     * @ghidraAddress PAL: 0x0012d600
     */
    int CountGems(int nStartBar, int nEndBar);

    /**
     * Write the gems of the sections that repeat no pattern.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0012be60
     * @ghidraAddress PAL: 0x0012d658
     */
    void Save(BinStream &stream);

    /**
     * Replace every gem with the gems Save() wrote.
     *
     * The gems read have no owner, and every section loses its owner.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0012c1e0
     * @ghidraAddress PAL: 0x0012d9c8
     */
    void Load(BinStream &stream);

private:
    /**
     * Count a gem toward the owner of the section of its bar.
     *
     * @param nOwner The player who placed the gem, or -1 to count nothing.
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x0012c5c0
     * @ghidraAddress PAL: 0x0012dd98
     */
    void AddOwner(int nOwner, int nBar);

    /**
     * Withdraw a gem from the owner of the section of its bar.
     *
     * The section loses its owner when its last counted gem goes.
     *
     * @param nOwner The player who placed the gem, or -1 to withdraw nothing.
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x0012c620
     * @ghidraAddress PAL: 0x0012ddf8
     */
    void RemoveOwner(int nOwner, int nBar);

    int mTicksPerBar;                         /*!< The song ticks in one bar. */
    SectionBoundaries *mSections;             /*!< The sections of the song. */
    SlotGrid *mPatterns;                      /*!< The pattern of each section, or null. */
    std::vector<SecStats> mSecStats;          /*!< The owner of each section. */
    std::vector<std::vector<PitchGem>> mBars; /*!< The gems of each bar, in tick order. */
};

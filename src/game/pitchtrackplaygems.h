#pragma once

#include "game/gemslice.h"
#include "game/pitchtrackgems.h"
#include "game/playmap.h"
#include "game/sectionboundaries.h"
#include "game/slotgrid.h"
#include "os/binstream.h"

/**
 * The gems of a pitch track, addressed by the bars and ticks the song plays.
 *
 * The RTTI includes the class name. The class is not polymorphic. The member names are inferred.
 */
class PitchTrackPlayGems {
public:
    /**
     * Construct a view of gems.
     *
     * @param pGems The gems.
     * @param pPlayMap The map of the song positions.
     * @param pSections The sections of the song.
     * @param pPatterns The pattern each section repeats, or null.
     * @param nNumBars The length of the song in bars. It is unused.
     * @param nTicksPerBar The song ticks in one bar.
     * @ghidraAddress NTSC-U/C: 0x0012c688
     * @ghidraAddress PAL: 0x0012de60
     */
    PitchTrackPlayGems(PitchTrackGems *pGems,
                       PlayMap *pPlayMap,
                       SectionBoundaries *pSections,
                       SlotGrid *pPatterns,
                       int nNumBars,
                       int nTicksPerBar);

    /**
     * Release the view. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x0012c6a8
     * @ghidraAddress PAL: 0x0012de80
     */
    ~PitchTrackPlayGems();

    /**
     * Place or remove the gem at a tick, and on request at the same tick of every other bar of the
     * section.
     *
     * @param nSlot The gem button.
     * @param nTick The tick the song plays at.
     * @param nOwner The player who edits the gem.
     * @param bRepeat Whether the edit repeats on every other bar of the section.
     * @return Whether a gem lies at the tick afterwards.
     * @ghidraAddress NTSC-U/C: 0x0012c6d0
     * @ghidraAddress PAL: 0x0012dea8
     */
    bool ToggleGem(signed char nSlot, int nTick, signed char nOwner, bool bRepeat);

    /**
     * Report the owner of the section a bar lies in.
     *
     * @param nBar The bar the song plays.
     * @return The player, or -1.
     * @ghidraAddress NTSC-U/C: 0x0012c830
     * @ghidraAddress PAL: 0x0012e008
     */
    int GetOwner(int nBar);

    /**
     * Remove every gem of a bar, and on request of every other bar of the section.
     *
     * @param nBar The bar the song plays.
     * @param bRepeat Whether every other bar of the section is cleared.
     * @ghidraAddress NTSC-U/C: 0x0012c868
     * @ghidraAddress PAL: 0x0012e040
     */
    void ClearBar(int nBar, bool bRepeat);

    /**
     * Remove every gem of the section a bar lies in.
     *
     * @param nBar The bar the song plays.
     * @ghidraAddress NTSC-U/C: 0x0012c938
     * @ghidraAddress PAL: 0x0012e110
     */
    void ClearSection(int nBar);

    /**
     * Copy the gems of a range of bars of other gems.
     *
     * @param pSource The gems copied.
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x0012c980
     * @ghidraAddress PAL: 0x0012e158
     */
    void CopyBars(PitchTrackPlayGems *pSource, int nStartBar, int nEndBar);

    /**
     * Copy the gems of each section's pattern into the section.
     *
     * @ghidraAddress NTSC-U/C: 0x0012c9b0
     * @ghidraAddress PAL: 0x0012e188
     */
    void ApplyPatterns();

    /**
     * Report the gems of a bar the song plays.
     *
     * @param nBar The bar the song plays.
     * @return The gems, or a view of no gems inside a stretch of silence.
     * @ghidraAddress NTSC-U/C: 0x0012c9d0
     * @ghidraAddress PAL: 0x0012e1a8
     */
    GemSlice GetBar(int nBar);

    /**
     * Write the gems.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0012ca58
     * @ghidraAddress PAL: 0x0012e230
     */
    void Save(BinStream &stream);

    /**
     * Read the gems.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0012ca78
     * @ghidraAddress PAL: 0x0012e250
     */
    void Load(BinStream &stream);

private:
    PitchTrackGems *mGems;        /*!< The gems. */
    PlayMap *mPlayMap;            /*!< The map of the song positions. */
    SectionBoundaries *mSections; /*!< The sections of the song. */
    SlotGrid *mPatterns;          /*!< The pattern each section repeats, or null. */
    int mTicksPerBar;             /*!< The song ticks in one bar. */
};

#pragma once

#include "game/pitchtrackplaygems.h"
#include "game/playmap.h"
#include "game/sectionboundaries.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Display of the bars and gems of a pitch track on the play field, kept a stretch of ticks ahead
 * of the song.
 *
 * The RTTI records the command the class schedules. The class is not polymorphic. The member names
 * are inferred.
 */
class PitchTrackDisplay {
public:
    /**
     * Construct a display.
     *
     * @param nTrack The track.
     * @param nLeadInBars The bars the song plays before bar 0.
     * @param nTicksPerBar The song ticks in one bar.
     * @param pPlayMap The map of the song positions.
     * @param pSections The sections of the song.
     * @param pGems The gems shown.
     * @ghidraAddress NTSC-U/C: 0x00129f50
     * @ghidraAddress PAL: 0x0012b748
     */
    PitchTrackDisplay(int nTrack,
                      int nLeadInBars,
                      int nTicksPerBar,
                      PlayMap *pPlayMap,
                      SectionBoundaries *pSections,
                      PitchTrackPlayGems *pGems);

    /**
     * Withdraw the scheduled command and release the display.
     *
     * @ghidraAddress NTSC-U/C: 0x00129fc8
     * @ghidraAddress PAL: 0x0012b7c0
     */
    ~PitchTrackDisplay();

    /**
     * Show the bars ahead of the song and schedule the next update.
     *
     * @ghidraAddress NTSC-U/C: 0x0012a018
     * @ghidraAddress PAL: 0x0012b810
     */
    void Start();

    /**
     * Withdraw the scheduled update.
     *
     * @ghidraAddress NTSC-U/C: 0x0012a038
     * @ghidraAddress PAL: 0x0012b830
     */
    void Stop();

    /**
     * Show other gems from the current bar on.
     *
     * @param pGems The gems.
     * @param nStyle The style of the gems.
     * @param nUnused Unused.
     * @param nOwner The owner every gem is shown with, or -1 for the owner of each gem.
     * @ghidraAddress NTSC-U/C: 0x0012a070
     * @ghidraAddress PAL: 0x0012b868
     */
    void SetGems(PitchTrackPlayGems *pGems, int nStyle, int nUnused, int nOwner);

    /**
     * Show a range of bars empty, outside the song.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x0012a0c0
     * @ghidraAddress PAL: 0x0012b8b8
     */
    void BlankBars(int nStartBar, int nEndBar);

    /**
     * Show a gem placed.
     *
     * @param nSlot The gem button.
     * @param nTick The tick.
     * @param bRepeat Whether the gem repeats on every other bar of the section.
     * @param nOwner The player who placed the gem.
     * @ghidraAddress NTSC-U/C: 0x0012a1a8
     * @ghidraAddress PAL: 0x0012b9a0
     */
    void ShowGem(signed char nSlot, int nTick, bool bRepeat, int nOwner);

    /**
     * Show a gem removed.
     *
     * @param nTick The tick.
     * @param bRepeat Whether the gem repeats on every other bar of the section.
     * @ghidraAddress NTSC-U/C: 0x0012a1c8
     * @ghidraAddress PAL: 0x0012b9c0
     */
    void HideGem(int nTick, bool bRepeat);

    /**
     * Show again the gems from a bar to the last bar shown.
     *
     * @param nStartBar The first bar.
     * @ghidraAddress NTSC-U/C: 0x0012a1f8
     * @ghidraAddress PAL: 0x0012b9f0
     */
    void Redraw(int nStartBar);

    /**
     * Show again the gems of a range of bars, ending no later than the last bar shown.
     *
     * @param nStartBar The first bar.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x0012a218
     * @ghidraAddress PAL: 0x0012ba10
     */
    void Redraw(int nStartBar, int nEndBar);

    /**
     * Remove the gems shown from a bar to the last bar shown.
     *
     * @param nStartBar The first bar.
     * @ghidraAddress NTSC-U/C: 0x0012a258
     * @ghidraAddress PAL: 0x0012ba50
     */
    void ClearGems(int nStartBar);

    /**
     * Remove the gems shown in a range of bars.
     *
     * @param nStartBar The first bar.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x0012a278
     * @ghidraAddress PAL: 0x0012ba70
     */
    void ClearGems(int nStartBar, int nEndBar);

    /**
     * Show a bar again, and each later bar shown that plays the same bar of the song.
     *
     * @param nBar The bar.
     * @param bClear Whether the gems of the bars are removed rather than shown again.
     * @ghidraAddress NTSC-U/C: 0x0012a2c8
     * @ghidraAddress PAL: 0x0012bac0
     */
    void RedrawBar(int nBar, bool bClear);

    /**
     * Remove the gems of a bar and of every other bar after it shown in the same section.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x0012a3c8
     * @ghidraAddress PAL: 0x0012bbc0
     */
    void ClearRepeatedBars(int nBar);

    /**
     * Remove the gems of a bar, and on request of every other bar after it in the same section.
     *
     * @param nBar The bar.
     * @param bRepeat Whether every other bar of the section is cleared.
     * @ghidraAddress NTSC-U/C: 0x0012a468
     * @ghidraAddress PAL: 0x0012bc60
     */
    void ClearBar(int nBar, bool bRepeat);

private:
    /**
     * Show a gem placed or removed, and on request at the same tick of every other bar after it
     * shown in the same section.
     *
     * @param nSlot The gem button.
     * @param nTick The tick.
     * @param bRepeat Whether the gem repeats on every other bar of the section.
     * @param nOwner The player who placed the gem.
     * @param bPlace Whether the gem is placed rather than removed.
     * @ghidraAddress NTSC-U/C: 0x0012a498
     * @ghidraAddress PAL: 0x0012bc90
     */
    void DrawGem(signed char nSlot, int nTick, bool bRepeat, int nOwner, bool bPlace);

    /**
     * Show the bars ahead of the song and schedule the next update a bar later.
     *
     * @ghidraAddress NTSC-U/C: 0x0012a670
     * @ghidraAddress PAL: 0x0012be68
     */
    void Update();

    /**
     * Show a range of bars and their gems.
     *
     * @param nStartBar The first bar.
     * @param nEndBar The bar after the range.
     * @param bClear Whether the gems shown in the range are removed first.
     * @ghidraAddress NTSC-U/C: 0x0012a6f8
     * @ghidraAddress PAL: 0x0012bef0
     */
    void DrawRange(int nStartBar, int nEndBar, bool bClear);

    /**
     * Show a range of bars.
     *
     * @param nStartBar The first bar.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x0012a760
     * @ghidraAddress PAL: 0x0012bf58
     */
    void DrawBars(int nStartBar, int nEndBar);

    /**
     * Show the gems of a range of bars.
     *
     * @param nStartBar The first bar, raised to 0.
     * @param nEndBar The bar after the range.
     * @param bClear Whether the gems shown in the range are removed first.
     * @param nUnused Unused.
     * @param nOwner The owner every gem is shown with, or -1 for the owner of each gem.
     * @ghidraAddress NTSC-U/C: 0x0012a898
     * @ghidraAddress PAL: 0x0012c090
     */
    void DrawGems(int nStartBar, int nEndBar, bool bClear, int nUnused, int nOwner);

    int mTrack;                   /*!< The track. */
    int mTicksPerBar;             /*!< The song ticks in one bar. */
    PlayMap *mPlayMap;            /*!< The map of the song positions. */
    SectionBoundaries *mSections; /*!< The sections of the song. */
    PitchTrackPlayGems *mGems;    /*!< The gems shown. */
    Ptr<Command> mUpdateCommand;  /*!< The command that calls Update(). */
    int mEndBar;                  /*!< The bar after the last bar shown. */
    int mStyle;                   /*!< The style of the gems. */
};

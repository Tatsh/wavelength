#pragma once

#include "game/catchtrackdata.h"
#include "game/playmap.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Display of the bars and gems of a vocal track on the play field, kept a stretch of ticks ahead
 * of the song.
 *
 * The RTTI records the command the class schedules. The class is not polymorphic. The member names
 * are inferred.
 */
class VoxTrackDisplay {
public:
    /**
     * Construct a display.
     *
     * @param nTrack The track.
     * @param nLeadInBars The bars the song plays before bar 0.
     * @param nTicksPerBar The song ticks in one bar.
     * @param pPlayMap The map of the song positions.
     * @param pGems The gems shown.
     * @ghidraAddress NTSC-U/C: 0x00143558
     * @ghidraAddress PAL: 0x00144ee8
     */
    VoxTrackDisplay(
        int nTrack, int nLeadInBars, int nTicksPerBar, PlayMap *pPlayMap, CatchTrackData *pGems);

    /**
     * Withdraw the scheduled command and release the display.
     *
     * @ghidraAddress NTSC-U/C: 0x001435c8
     * @ghidraAddress PAL: 0x00144f58
     */
    ~VoxTrackDisplay();

    /**
     * Show the bars up to the lookahead, and keep them shown as the song plays.
     *
     * @ghidraAddress NTSC-U/C: 0x00143618
     * @ghidraAddress PAL: 0x00144fa8
     */
    void Start();

    /**
     * Withdraw the scheduled command.
     *
     * @ghidraAddress NTSC-U/C: 0x00143638
     * @ghidraAddress PAL: 0x00144fc8
     */
    void Stop();

    /**
     * Show a range of bars empty.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x00143670
     * @ghidraAddress PAL: 0x00145000
     */
    void BlankBars(int nStartBar, int nEndBar);

    /**
     * Show the bars and gems again from a bar to the last bar shown.
     *
     * @param nStartBar The first bar.
     * @ghidraAddress NTSC-U/C: 0x00143758
     * @ghidraAddress PAL: 0x001450e8
     */
    void Redraw(int nStartBar);

    /**
     * Show the bars and gems again over a range, cut at the last bar shown.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x00143778
     * @ghidraAddress PAL: 0x00145108
     */
    void Redraw(int nStartBar, int nEndBar);

private:
    /**
     * Show the bars up to the lookahead, and schedule the next update a bar later.
     *
     * @ghidraAddress NTSC-U/C: 0x001437b8
     * @ghidraAddress PAL: 0x00145148
     */
    void Update();

    /**
     * Show the bars and gems of a range.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @param bClear Whether the gems shown in the range are removed first.
     * @ghidraAddress NTSC-U/C: 0x00143840
     * @ghidraAddress PAL: 0x001451d0
     */
    void DrawRange(int nStartBar, int nEndBar, bool bClear);

    /**
     * Show the bars of a range, the bars before bar 0 empty.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x001438a0
     * @ghidraAddress PAL: 0x00145230
     */
    void DrawBars(int nStartBar, int nEndBar);

    /**
     * Show the gems of a range from bar 0 on.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @param bClear Whether the gems shown in the range are removed first.
     * @ghidraAddress NTSC-U/C: 0x00143990
     * @ghidraAddress PAL: 0x00145320
     */
    void DrawGems(int nStartBar, int nEndBar, bool bClear);

    int mTrack;                  /*!< The track. */
    int mTicksPerBar;            /*!< The song ticks in one bar. */
    PlayMap *mPlayMap;           /*!< The map of the song positions. */
    CatchTrackData *mGems;       /*!< The gems shown. */
    Ptr<Command> mUpdateCommand; /*!< The command that calls Update(). */
    int mEndBar;                 /*!< The bar after the last bar shown. */
};

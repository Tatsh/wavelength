#pragma once

#include <vector>

#include "game/catchtrackdata.h"
#include "game/gemcursor.h"
#include "game/player.h"
#include "game/playmap.h"

/**
 * Per-bar state of one catch track during a song.
 *
 * The RTTI includes the nested CatchTrackState::BarState. Only the members its callers here use
 * are declared.
 */
class CatchTrackState {
public:
    /**
     * State of one bar of the song as written.
     *
     * The RTTI includes the class name.
     */
    struct BarState {
        Player *mCapturedBy; /*!< The player who captured the phrase of the bar, or null. */
        int mEnabled;        /*!< Whether the gems of the bar can be caught. */
        int mHasGems;        /*!< Whether a gem of the track lies in the bar. */
        int mPowerup;        /*!< The power-up the phrase of the bar awards, or 0. */
    };

    /**
     * Construct the state of every bar and mark the bars that have gems.
     *
     * @param pData The gems of the track.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The song ticks in one bar.
     * @param pPlayMap The play map of the song.
     * @ghidraAddress NTSC-U/C: 0x0014e000
     * @ghidraAddress PAL: 0x0014f9a0
     */
    CatchTrackState(CatchTrackData *pData, int nNumBars, int nTicksPerBar, PlayMap *pPlayMap);

    /**
     * Release the state of the bars.
     *
     * @ghidraAddress NTSC-U/C: 0x0014e100
     * @ghidraAddress PAL: 0x0014faa0
     */
    ~CatchTrackState();

    /**
     * Report the player who captured the phrase of a bar.
     *
     * @param nBar The bar the song plays.
     * @return The player, or null while the phrase is not captured.
     * @ghidraAddress NTSC-U/C: 0x0014e230
     * @ghidraAddress PAL: 0x0014fbd0
     */
    Player *GetCapturedBy(int nBar);

    /**
     * Set whether the gems of a range of bars can be caught.
     *
     * @param nStartBar The first bar the song plays.
     * @param nEndBar The bar after the range.
     * @param bEnabled Whether the gems can be caught.
     * @ghidraAddress NTSC-U/C: 0x0014e270
     * @ghidraAddress PAL: 0x0014fc10
     */
    void SetEnabled(int nStartBar, int nEndBar, bool bEnabled);

    /**
     * Report whether the gems of a bar can be caught.
     *
     * @param nBar The bar the song plays.
     * @return Whether the gems can be caught.
     * @ghidraAddress NTSC-U/C: 0x0014e2e8
     * @ghidraAddress PAL: 0x0014fc88
     */
    bool IsEnabled(int nBar);

    /**
     * Report whether no gem of the track lies in a bar.
     *
     * @param nBar The bar the song plays.
     * @return Whether the bar has no gems.
     * @ghidraAddress NTSC-U/C: 0x0014e320
     * @ghidraAddress PAL: 0x0014fcc0
     */
    bool IsBarEmpty(int nBar);

    /**
     * Report the power-up a bar awards when its phrase is caught.
     *
     * @param nBar The bar the song plays.
     * @return The kind of power-up, or 0 for none.
     * @ghidraAddress NTSC-U/C: 0x0014e390
     * @ghidraAddress PAL: 0x0014fd30
     */
    int GetPowerup(int nBar);

    /**
     * Construct a cursor at the first gem of the track.
     *
     * @return The cursor.
     * @ghidraAddress NTSC-U/C: 0x0014e428
     * @ghidraAddress PAL: 0x0014fdc8
     */
    GemCursor GetCursor();

    /**
     * Construct a cursor at the first gem of a lane at or after a tick.
     *
     * @param nLane The lane.
     * @param nTick The tick the song plays at.
     * @return The cursor.
     * @ghidraAddress NTSC-U/C: 0x0014e458
     * @ghidraAddress PAL: 0x0014fdf8
     */
    GemCursor GetCursor(int nLane, int nTick);

    /**
     * Construct a cursor at the first gem of any lane at or after a tick.
     *
     * @param nTick The tick the song plays at.
     * @return The cursor.
     * @ghidraAddress NTSC-U/C: 0x0014e480
     * @ghidraAddress PAL: 0x0014fe20
     */
    GemCursor GetCursor(int nTick);

    /**
     * Disable a range of bars and remove the gems the range has.
     *
     * @param nStartBar The first bar the song plays.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x0014e560
     * @ghidraAddress PAL: 0x0014ff00
     */
    void ClearBars(int nStartBar, int nEndBar);

private:
    PlayMap *mPlayMap;           /*!< The play map of the song. */
    CatchTrackData *mData;       /*!< The gems of the track. */
    int mNumBars;                /*!< The length of the song in bars. */
    int mTicksPerBar;            /*!< The song ticks in one bar. */
    std::vector<BarState> mBars; /*!< The state of each bar of the song as written. */
};

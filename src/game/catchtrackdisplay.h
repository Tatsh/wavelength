#pragma once

#include "game/catchtrackstate.h"
#include "game/playmap.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Drawer of the bars and gems of one catch track ahead of the song.
 *
 * The RTTI includes the nested CatchTrackDisplay::UpdateDisplayCmd. The class is not polymorphic.
 * CatchTrack has one. Once each bar the display draws the bars that come within 13000 ticks of
 * the current tick, the updates of each track a tenth of a bar apart.
 */
class CatchTrackDisplay {
public:
    /**
     * Command that draws the bars that come into view and schedules itself a bar later.
     *
     * The RTTI includes the nested name and records Command as the base. Its members are inline.
     */
    class UpdateDisplayCmd : public Command {
    public:
        /**
         * Construct the command of a display.
         *
         * @param pDisplay The display.
         */
        explicit UpdateDisplayCmd(CatchTrackDisplay *pDisplay) : mDisplay(pDisplay) {
        }

        /**
         * Draw the bars that come into view, and run again a bar later until the song ends.
         *
         * @ghidraAddress NTSC-U/C: 0x0034d4d8
         * @ghidraAddress PAL: 0x003ba908
         */
        void Execute() override;

    private:
        CatchTrackDisplay *mDisplay; /*!< The display. */
    };

    /**
     * Construct a display that has drawn nothing.
     *
     * @param pState The state of the bars of the track.
     * @param nReserved04 Stored and not read here.
     * @param nTrack The track.
     * @param nReserved0C Stored and not read here.
     * @param nReserved10 Stored and not read here.
     * @param nTicksPerBar The song ticks in one bar.
     * @param pPlayMap The play map of the song.
     * @ghidraAddress NTSC-U/C: 0x001579f0
     * @ghidraAddress PAL: 0x00159278
     */
    CatchTrackDisplay(CatchTrackState *pState,
                      int nReserved04,
                      int nTrack,
                      int nReserved0C,
                      int nReserved10,
                      int nTicksPerBar,
                      PlayMap *pPlayMap);

    /**
     * Withdraw the scheduled update.
     *
     * @ghidraAddress NTSC-U/C: 0x00157a78
     * @ghidraAddress PAL: 0x00159300
     */
    ~CatchTrackDisplay();

    /**
     * Schedule the next update.
     *
     * @ghidraAddress NTSC-U/C: 0x00157b50
     * @ghidraAddress PAL: 0x001593d8
     */
    void Start();

    /**
     * Move the display to a tick, drawing the bars that come into view from it.
     *
     * @param nTick The tick the song plays from.
     * @ghidraAddress NTSC-U/C: 0x00157b80
     * @ghidraAddress PAL: 0x00159408
     */
    void Seek(int nTick);

    /**
     * Withdraw the scheduled update.
     *
     * @ghidraAddress NTSC-U/C: 0x00157bf0
     * @ghidraAddress PAL: 0x00159478
     */
    void Stop();

    /**
     * Draw again the bars from one bar up to the last bar drawn.
     *
     * @param nFromBar The first bar.
     * @param nClear Non-zero to remove the gems of the bars first.
     * @ghidraAddress NTSC-U/C: 0x00157c28
     * @ghidraAddress PAL: 0x001594b0
     */
    void Redraw(int nFromBar, int nClear);

    /**
     * Draw again a range of bars, limited to the bars drawn so far.
     *
     * @param nFromBar The first bar.
     * @param nToBar The bar after the range.
     * @param nClear Non-zero to remove the gems of the bars first.
     * @ghidraAddress NTSC-U/C: 0x00157c48
     * @ghidraAddress PAL: 0x001594d0
     */
    void RedrawRange(int nFromBar, int nToBar, int nClear);

private:
    /**
     * Schedule the next update for the start of the next bar, delayed by the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00157ac8
     * @ghidraAddress PAL: 0x00159350
     */
    void ScheduleUpdate();

    /**
     * Place the gems of a range of bars that can be caught or are captured.
     *
     * @param nFromBar The first bar.
     * @param nToBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x00157d08
     * @ghidraAddress PAL: 0x00159590
     */
    void PlaceGems(int nFromBar, int nToBar);

    /**
     * Draw a range of bars and place their gems.
     *
     * @param nFromBar The first bar.
     * @param nToBar The bar after the range.
     * @param nVisible Non-zero to show the bars.
     * @ghidraAddress NTSC-U/C: 0x00157e78
     * @ghidraAddress PAL: 0x00159700
     */
    void DrawBars(int nFromBar, int nToBar, int nVisible);

    /**
     * Draw the bars that came into view since the last update.
     *
     * @return Whether bars remain to be drawn.
     * @ghidraAddress NTSC-U/C: 0x00158010
     * @ghidraAddress PAL: 0x00159898
     */
    bool Advance();

    CatchTrackState *mState;          /*!< The state of the bars of the track. */
    int mReserved04;                  // +0x04, stored by the constructor, not read here.
    int mTrack;                       /*!< The track. */
    int mReserved0C;                  // +0x0c, stored by the constructor, not read here.
    int mReserved10;                  // +0x10, stored by the constructor, not read here.
    int mTicksPerBar;                 /*!< The song ticks in one bar. */
    Ptr<UpdateDisplayCmd> mUpdateCmd; /*!< The command that calls Advance(). */
    int mDrawnBar;                    /*!< The bar after the bars drawn so far. */
    int mTickOffset;                  /*!< The ticks between the song and the display. */
    PlayMap *mPlayMap;                /*!< The play map of the song. */
};

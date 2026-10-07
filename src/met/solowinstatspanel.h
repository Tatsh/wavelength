#pragma once

#include "met/sologamestatspanel.h"
#include "met/viewanimplayer.h"
#include "script/dataarray.h"

/**
 * Panel that shows the statistics of a solo song just won, against the player's best.
 *
 * The RTTI records the class as deriving from SoloGameStatsPanel. The object is 0x100 bytes and its
 * vtables are at `0x003cc5e8` and, for StatsPanel, `0x003cc5d0`. A remix shows no best and no
 * grade. The panel's view `<name>.view` reveals the statistics with the `TEXT` sound.
 */
class SoloWinStatsPanel : public SoloGameStatsPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the panel's files.
     * @ghidraAddress NTSC-U/C: 0x0016fac8
     * @ghidraAddress PAL: 0x00172dd0
     */
    SoloWinStatsPanel(DataArray *pData, const char *pszDir);

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the panel's files.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003575e0
     * @ghidraAddress PAL: 0x003c4840
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new SoloWinStatsPanel(pData, pszDir);
    }

    /**
     * Unload the panel and forget the view of the reveal.
     *
     * @ghidraAddress NTSC-U/C: 0x0016fb10
     * @ghidraAddress PAL: 0x00172e18
     */
    void Unload() override;

    /**
     * Exit, and stop the reveal.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016fb40
     * @ghidraAddress PAL: 0x00172e48
     */
    void Exit(bool bForce, float fTime) override;

    /**
     * Show the statistics of the song, the player's best, and the grade.
     *
     * @ghidraAddress NTSC-U/C: 0x0016fc00
     * @ghidraAddress PAL: 0x00172f08
     */
    void Refresh() override;

    /**
     * Advance the panel and the reveal.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016ffe0
     * @ghidraAddress PAL: 0x001732e8
     */
    void Poll(float fTime) override;

    /**
     * Start the reveal.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00170020
     * @ghidraAddress PAL: 0x00173328
     */
    void StartAnim(float fTime);

    ViewAnimPlayer mReveal; /*!< The reveal of the statistics. */
};

#pragma once

#include "met/freqpanel.h"
#include "met/statspanel.h"
#include "script/dataarray.h"

/**
 * Panel that shows the band, the song, and the skill level of the solo song just played.
 *
 * The RTTI records the class as deriving from FreqPanel and from StatsPanel at `+0xe0`. The object
 * is 0xf0 bytes and its vtables are at `0x003cc7b0` and, for StatsPanel, `0x003cc798`. The
 * statistics panels of a won and of a lost song derive from the class.
 */
class SoloGameStatsPanel : public FreqPanel, public StatsPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the panel's files.
     * @ghidraAddress NTSC-U/C: 0x0016f8a0
     * @ghidraAddress PAL: 0x00172ba8
     */
    SoloGameStatsPanel(DataArray *pData, const char *pszDir);

    /**
     * Show the short artist, the short title, and the skill level of the song.
     *
     * @ghidraAddress NTSC-U/C: 0x0016f8e0
     * @ghidraAddress PAL: 0x00172be8
     */
    void Refresh() override;

    /**
     * Show text on a label of the panel.
     *
     * @param pszComponent The label.
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x0016fb70
     * @ghidraAddress PAL: 0x00172e78
     */
    void SetLabel(const char *pszComponent, const char *pszText);
};

#pragma once

#include "met/sologamestatspanel.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Statistics panel after a lost solo game, which adds the part of the song played.
 *
 * The RTTI records the class as deriving from SoloGameStatsPanel.
 */
class SoloLoseStatsPanel : public SoloGameStatsPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00170040
     * @ghidraAddress PAL: 0x00173348
     */
    SoloLoseStatsPanel(DataArray *pData, const char *pszDir) : SoloGameStatsPanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003573e8
     * @ghidraAddress PAL: 0x003c4648
     */
    ~SoloLoseStatsPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new SoloLoseStatsPanel(pData, pszDir);
    }

    /**
     * Show the statistics and the percentage of the song played.
     *
     * @ghidraAddress NTSC-U/C: 0x00170080
     * @ghidraAddress PAL: 0x00173388
     */
    void Refresh() override;
};

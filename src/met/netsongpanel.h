#pragma once

#include "met/freqpanel.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Tabbed panel with a `list`, whose tab and background light while the panel has the focus.
 *
 * The RTTI records the class as deriving from FreqPanel.
 */
class NetSongPanel : public FreqPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00173cd8
     * @ghidraAddress PAL: 0x00177138
     */
    NetSongPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00358360
     * @ghidraAddress PAL: 0x003c5710
     */
    ~NetSongPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003583f8
     * @ghidraAddress PAL: 0x003c57a8
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetSongPanel(pData, pszDir);
    }

    /**
     * Take the focus, show the cursor of the list, and light the tab and the background.
     *
     * The panel does nothing while it is not loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00173d10
     * @ghidraAddress PAL: 0x00177170
     */
    void Focus() override;

    /**
     * Dim the cursor of the list, the tab, and the background.
     *
     * The panel does nothing while it is not loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00173ef0
     * @ghidraAddress PAL: 0x00177350
     */
    void Unfocus() override;
};

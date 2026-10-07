#pragma once

#include "met/freqpanel.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel that shows the title, the artist, and the label of the song of the game.
 *
 * The RTTI records the class as deriving from FreqPanel.
 */
class LaunchMTVPanel : public FreqPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00171710
     * @ghidraAddress PAL: 0x00174a18
     */
    LaunchMTVPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00357988
     * @ghidraAddress PAL: 0x003c4be8
     */
    ~LaunchMTVPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new LaunchMTVPanel(pData, pszDir);
    }

    /**
     * Label the song and enter.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00171748
     * @ghidraAddress PAL: 0x00174a50
     */
    void Enter(bool bForce, float fTime) override;
};

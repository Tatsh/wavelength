#pragma once

#include "met/freqpanel.h"
#include "met/netflashupdate.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel of the full ranking, whose mesh flashes when the ranking changes.
 *
 * The RTTI records the class as deriving from FreqPanel and from NetFlashUpdate.
 */
class FullRankedPanel : public FreqPanel, public NetFlashUpdate {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00172a68
     * @ghidraAddress PAL: 0x00175eb0
     */
    FullRankedPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00357e90
     * @ghidraAddress PAL: 0x003c5240
     */
    ~FullRankedPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new FullRankedPanel(pData, pszDir);
    }

    /**
     * Advance the panel and the flash of its mesh.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00172ae8
     * @ghidraAddress PAL: 0x00175f30
     */
    void Poll(float fTime) override;

    /**
     * Finish loading and find the objects of the flash, with the `sub` materials.
     *
     * @ghidraAddress NTSC-U/C: 0x00172aa8
     * @ghidraAddress PAL: 0x00175ef0
     */
    void FinishLoad() override;
};

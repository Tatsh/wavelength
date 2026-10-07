#pragma once

#include "met/freqpanel.h"
#include "rnd/rndloader.h"
#include "script/dataarray.h"

/**
 * Panel that shows the bonus picture of a song, which loads from `Songs\<song>\bonus.rnd` with the
 * panel.
 *
 * The RTTI records the class as deriving from FreqPanel. The object is 0xf0 bytes and its vtable
 * is at `0x003cf368`. The front-end description's `s_g_bonus_pic` panel is one. The destructor at
 * `0x0035ebe0` is compiler-generated and has no declaration here.
 */
class BonusPicPanel : public FreqPanel {
public:
    /**
     * Construct the panel from its script description, with no song.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x0035ecc8
     * @ghidraAddress PAL: 0x003ccda8
     */
    BonusPicPanel(DataArray *pData, const char *pszDir);

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x0035ec78
     * @ghidraAddress PAL: 0x003ccd58
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new BonusPicPanel(pData, pszDir);
    }

    /**
     * Start loading the bonus picture of mSong with the first load of the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00198430
     * @ghidraAddress PAL: 0x0019f950
     */
    void Load() override;

    /**
     * Release the bonus picture with the last unload of the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00198490
     * @ghidraAddress PAL: 0x0019f9b0
     */
    void Unload() override;

    const char *mSong;       /*!< The song whose picture shows, a symbol. +0xe0 */
    RndLoader *mBonusLoader; /*!< The loader of the bonus picture, or null. */
};

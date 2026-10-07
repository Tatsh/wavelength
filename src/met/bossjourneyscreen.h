#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"

/**
 * Screen that shows while the journey to the boss arena plays, and shows the next queued unlock
 * once it is done.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x80 bytes and its vtable
 * is at `0x003cf1e8`. The destructor at `0x0035f078` is compiler-generated and has no declaration
 * here.
 */
class BossJourneyScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0035f150
     * @ghidraAddress PAL: 0x003cd230
     */
    explicit BossJourneyScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035f110
     * @ghidraAddress PAL: 0x003cd1f0
     */
    static UIScreen *New(DataArray *pData) {
        return new BossJourneyScreen(pData);
    }

    /**
     * Show the next queued unlock once the journey is done.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00198e68
     * @ghidraAddress PAL: 0x001a0388
     */
    void Poll(float fTime) override;

    /**
     * Start the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00198e48
     * @ghidraAddress PAL: 0x001a0368
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    int mDone; /*!< Set once the journey is done. */
};

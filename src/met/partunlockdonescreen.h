#pragma once

#include "met/unlockscreen.h"
#include "script/dataarray.h"

/**
 * Screen that ends the list of unlocked Freq parts, and shows the next queued unlock after ten
 * seconds or when the cross button goes down.
 *
 * The RTTI records the class as deriving from UnlockScreen. The object is 0x80 bytes and its
 * vtable is at `0x003cf3f0`. The metagame registers the class for the screen type
 * `parts_unlock_done_screen`. The destructor at `0x0035eac0` is compiler-generated and has no
 * declaration here.
 */
class PartUnlockDoneScreen : public UnlockScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001982c0
     * @ghidraAddress PAL: 0x0019f7e0
     */
    explicit PartUnlockDoneScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `parts_unlock_done_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035eba0
     * @ghidraAddress PAL: 0x003ccc80
     */
    static UIScreen *New(DataArray *pData) {
        return new PartUnlockDoneScreen(pData);
    }

    /**
     * Show the next queued unlock once the time has come.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00198390
     * @ghidraAddress PAL: 0x0019f8b0
     */
    void Poll(float fTime) override;

    /**
     * Start the entry and time the next queued unlock ten seconds later.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00198308
     * @ghidraAddress PAL: 0x0019f828
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    float mAdvanceMs; /*!< The system time the next queued unlock shows. */
};

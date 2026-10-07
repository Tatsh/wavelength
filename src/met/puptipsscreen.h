#pragma once

#include "met/tipsscreen.h"
#include "script/dataarray.h"

/**
 * Page of the tips about a power-up, whose first text shows the token `<panel>_p_only`.
 *
 * The RTTI records the class as deriving from TipsScreen. The object is 0x80 bytes and its vtable
 * is at `0x003ce840`. The destructor at `0x0035e560` is compiler-generated and has no declaration
 * here.
 */
class PupTipsScreen : public TipsScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0035e5b8
     * @ghidraAddress PAL: 0x003cc698
     */
    explicit PupTipsScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035e520
     * @ghidraAddress PAL: 0x003cc600
     */
    static UIScreen *New(DataArray *pData) {
        return new PupTipsScreen(pData);
    }

    /**
     * Show the page, then fill `<panel>_p_01.txt` of the focused panel when it exists.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00196450
     * @ghidraAddress PAL: 0x0019d820
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;
};

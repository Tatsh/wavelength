#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"

/**
 * Screen that shows while a song launches, and loads the launch dialog of the game ahead of it.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003ce9c0`. The tutorial loads `d_launch_training`, and any other game loads
 * `d_launch`. The destructor at `0x0035e0f8` is compiler-generated and has no declaration here.
 */
class LaunchScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00195c58
     * @ghidraAddress PAL: 0x0019d028
     */
    explicit LaunchScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035e190
     * @ghidraAddress PAL: 0x003cc270
     */
    static UIScreen *New(DataArray *pData) {
        return new LaunchScreen(pData);
    }

    /**
     * Load the launch dialog and start the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00195c90
     * @ghidraAddress PAL: 0x0019d060
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report no title.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035e1d0
     */
    const char *Title() override {
        return "";
    }
};

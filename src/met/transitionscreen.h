#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"

/**
 * Screen that shows while the projector flies between two menus, such as `start2main`.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003cbf48`. The metagame registers the class for the screen type `transition_screen`.
 * The screen has no panels of its own. A `screen_change` trigger of the metagame arena file
 * animates the flight and changes to the next menu when it ends. The destructor at `0x003553c8` is
 * compiler-generated and has no declaration here.
 */
class TransitionScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x003551b0
     * @ghidraAddress PAL: 0x003c2460
     */
    explicit TransitionScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `transition_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00354f68
     */
    static UIScreen *New(DataArray *pData) {
        return new TransitionScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x003553b8
     * @ghidraAddress PAL: 0x003c2668
     */
    const char *Title() override {
        return "";
    }
};

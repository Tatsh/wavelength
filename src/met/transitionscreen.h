#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"

/**
 * A screen shown while the gizmo moves from one screen to the next.
 *
 * The RTTI includes the name and records FreqScreen as the one base. The object is 0x70 bytes.
 * The metagame registers the class for the screen type `transition_screen`. Only the members the
 * metagame uses are declared.
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
    explicit TransitionScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Report the title of the screen, which a transition does not have.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x003553b8
     * @ghidraAddress PAL: 0x003c2668
     */
    const char *Title() override {
        return "";
    }

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `transition_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00354f68
     */
    static UIScreen *New(DataArray *pData) {
        return new TransitionScreen(pData);
    }
};

#pragma once

#include "met/setupnamescreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Screen where the player types the name of a player to find.
 *
 * The RTTI records the class as deriving from SetupNameScreen.
 */
class PlayerSearchSetupScreen : public SetupNameScreen {
public:
    /**
     * Construct a screen with no text.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0035aa40
     * @ghidraAddress PAL: 0x003c8590
     */
    explicit PlayerSearchSetupScreen(DataArray *pData) : SetupNameScreen(pData) {
    }

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035a8e0
     * @ghidraAddress PAL: 0x003c8430
     */
    ~PlayerSearchSetupScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x0035aa00
     * @ghidraAddress PAL: 0x003c8550
     */
    static UIScreen *New(DataArray *pData) {
        return new PlayerSearchSetupScreen(pData);
    }

    /**
     * Pass the name to NetDoPlayerSearchScreen and change to it.
     *
     * @ghidraAddress NTSC-U/C: 0x0017d9c0
     * @ghidraAddress PAL: 0x00181680
     */
    void Submit() override;
};

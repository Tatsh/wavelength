#pragma once

#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Memory card dialog `no_space_error`, whose message is the localised name of its focused panel
 * formatted with the name of the memory card slot and the space a save needs.
 *
 * The RTTI records the class as deriving from ErrorScreen. Its vtable is at `0x003d14c8`.
 */
class SpaceErrorScreen : public ErrorScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00362e90
     */
    explicit SpaceErrorScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00362e20
     */
    ~SpaceErrorScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00362de0
     */
    static UIScreen *New(DataArray *pData) {
        return new SpaceErrorScreen(pData);
    }

    /**
     * Enter and show the message with the name of the slot and the space needed in the dialog.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001aaae0
     * @ghidraAddress PAL: 0x001b2ab0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    int mSpace; /*!< The space the save needs, which ErrorScreen::ShowNoSpaceError() sets. */
};

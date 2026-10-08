#pragma once

#include "met/errorscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Memory card dialog about a named save, whose message is the localised name of its focused panel
 * formatted with the name of the save and the name of the memory card slot.
 *
 * The RTTI records the class as deriving from ErrorScreen. Its vtable is at `0x003d1540`.
 */
class NameErrorScreen : public ErrorScreen {
public:
    /**
     * Construct the screen from its script description, with no name.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00362cf0
     */
    explicit NameErrorScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00362c68
     */
    ~NameErrorScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00362c28
     */
    static UIScreen *New(DataArray *pData) {
        return new NameErrorScreen(pData);
    }

    /**
     * Enter and show the message with the name of the save and of the slot in the dialog.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001aabc0
     * @ghidraAddress PAL: 0x001b2b98
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    String mSaveName; /*!< The name of the save the dialog is about. */
};

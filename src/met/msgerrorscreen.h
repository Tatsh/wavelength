#pragma once

#include "met/errorscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Memory card dialog whose message is a locale token set before the dialog opens, formatted with
 * the name of the memory card slot.
 *
 * The RTTI records the class as deriving from ErrorScreen. The object is 0xc0 bytes and its vtable
 * is at `0x003d15b8`.
 */
class MsgErrorScreen : public ErrorScreen {
public:
    /**
     * Construct the screen from its script description, with no message.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00362b38
     */
    explicit MsgErrorScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00362ab0
     */
    ~MsgErrorScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00362a70
     */
    static UIScreen *New(DataArray *pData) {
        return new MsgErrorScreen(pData);
    }

    /**
     * Enter and show the localised message with the name of the slot in the dialog.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001aaca0
     * @ghidraAddress PAL: 0x001b2c78
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    String mMessage; /*!< The locale token of the message. */
};

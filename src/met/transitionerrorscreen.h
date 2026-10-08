#pragma once

#include "met/freqscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Screen of an error message that leads on through its transition table.
 *
 * The RTTI records the class as deriving from FreqScreen. Its vtable is at `0x003d1468`. The
 * front-end description's `lobby_error` and `lpad_error` screens are ones.
 */
class TransitionErrorScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no message.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001aad78
     * @ghidraAddress PAL: 0x001b2d50
     */
    explicit TransitionErrorScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00362ec8
     * @ghidraAddress PAL: 0x003d14c0
     */
    ~TransitionErrorScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00362f70
     * @ghidraAddress PAL: 0x003d1568
     */
    static UIScreen *New(DataArray *pData) {
        return new TransitionErrorScreen(pData);
    }

    /**
     * Enter and show the message in the dialog.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001aade0
     * @ghidraAddress PAL: 0x001b2db8
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    String mMessage; /*!< The message the screen shows. */
};

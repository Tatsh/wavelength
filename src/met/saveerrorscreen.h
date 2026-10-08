#pragma once

#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Memory card dialog of a save that would replace a saved copy, whose `replace` button returns to
 * the saving screen allowed to replace it.
 *
 * The RTTI records the class as deriving from ErrorScreen. The object is 0xa0 bytes and its vtable
 * is at `0x003d13f0`.
 */
class SaveErrorScreen : public ErrorScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001aae48
     * @ghidraAddress PAL: 0x001b2e20
     */
    explicit SaveErrorScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00362fc8
     */
    ~SaveErrorScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003630b8
     * @ghidraAddress PAL: 0x003d16b0
     */
    static UIScreen *New(DataArray *pData) {
        return new SaveErrorScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x003630f8
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the choice to HandleSelect().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001aae80
     * @ghidraAddress PAL: 0x001b2e58
     */
    bool DispatchPriv(Message *pMsg) override;

private:
    /**
     * Return to the start screen, allowed to replace the saved copy, when the cross button chooses
     * `replace`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x001aaee8
     * @ghidraAddress PAL: 0x001b2ec0
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};

#pragma once

#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The dialog that queries a guest whether to save the read-only remix it received.
 *
 * The RTTI records the class as deriving from ErrorScreen, and its vtable is at `0x003ce400`. The
 * metagame registers the class for the screen type `read_only_save_screen`.
 */
class ReadOnlySaveScreen : public ErrorScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018bdd0
     * @ghidraAddress PAL: 0x00192588
     */
    explicit ReadOnlySaveScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `read_only_save_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035cc30
     * @ghidraAddress PAL: 0x003cad08
     */
    static UIScreen *New(DataArray *pData) {
        return new ReadOnlySaveScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018be08
     * @ghidraAddress PAL: 0x001925c0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035cc70
     * @ghidraAddress PAL: 0x003cad48
     */
    const char *Title() override {
        return "";
    }

    /**
     * Go back to the launchpad for `no`, and to the save screen for every other button chosen with
     * the cross button.
     *
     * The save screen returns to the launchpad, or without a session to the pending network screen
     * of the metagame or to the lost session dialog.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018be70
     * @ghidraAddress PAL: 0x00192628
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};

#pragma once

#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The dialog that warns the host before a remix is shared read only.
 *
 * The RTTI records the class as deriving from ErrorScreen, and its vtable is at `0x003ce388`. The
 * metagame registers the class for the screen type `net_ro_warn_screen`.
 */
class NetReadOnlyWarnScreen : public ErrorScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018bc38
     * @ghidraAddress PAL: 0x001923f0
     */
    explicit NetReadOnlyWarnScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `net_ro_warn_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035cd38
     * @ghidraAddress PAL: 0x003cae10
     */
    static UIScreen *New(DataArray *pData) {
        return new NetReadOnlyWarnScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018bc70
     * @ghidraAddress PAL: 0x00192428
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035cd78
     * @ghidraAddress PAL: 0x003cae50
     */
    const char *Title() override {
        return "";
    }

    /**
     * Make the remix read only and go on to `load_remix` for `yes`, or go back to the question
     * for every other button chosen with the cross button.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018bcd8
     * @ghidraAddress PAL: 0x00192490
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};

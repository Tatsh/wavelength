#pragma once

#include "met/freqscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The dialog that queries whether others may change the remix that is shared.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x88 bytes and its vtable
 * is at `0x003ce478`. The metagame registers the class for the screen type
 * `read_only_check_screen`. The description provides `next_screen`, and optionally
 * `needs_share_warning`. With the warning, `net_share_read_only_warning` shows before a remix is
 * shared read only.
 */
class ReadOnlyCheckScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018ba28
     * @ghidraAddress PAL: 0x001921e0
     */
    explicit ReadOnlyCheckScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `read_only_check_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035cb28
     * @ghidraAddress PAL: 0x003cac00
     */
    static UIScreen *New(DataArray *pData) {
        return new ReadOnlyCheckScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018bac8
     * @ghidraAddress PAL: 0x00192280
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035cb68
     * @ghidraAddress PAL: 0x003cac40
     */
    const char *Title() override {
        return "";
    }

    /**
     * Record the answer chosen with the cross button in the remix and the game database, and go to
     * mNextScreen.
     *
     * `no` makes the remix read only.
     *
     * @param pMsg The message.
     * @return True for the cross button, otherwise the result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018bb30
     * @ghidraAddress PAL: 0x001922e8
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    String mNextScreen;     /*!< The `next_screen` entry of the description. */
    int mNeedsShareWarning; /*!< The `needs_share_warning` entry of the description. */
};

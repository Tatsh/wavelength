#pragma once

#ifdef VIDEO_STANDARD_PAL

#include "met/errorscreen.h"
#include "msg/message.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Dialog `multi_load_no_card` of a player's slot without a memory card while the Freqs of every
 * player load.
 *
 * The RTTI records the class as deriving from ErrorScreen. The object is 0xa0 bytes, as
 * ErrorScreen is, and its vtable is at `0x00467aa8`. The metagame registers the class for the
 * screen type `multi_no_card_screen`. Retry loads the slot again, and continue goes on to the
 * next slot.
 */
class MultiNoCardScreen : public ErrorScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress PAL: 0x001b3fa8
     */
    explicit MultiNoCardScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress PAL: 0x003d1ac8
     */
    ~MultiNoCardScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress PAL: 0x003d1bb8
     */
    static UIScreen *New(DataArray *pData) {
        return new MultiNoCardScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress PAL: 0x003d1bf8
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route a selection to HandleSelect().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress PAL: 0x001b3fe0
     */
    bool DispatchPriv(Message *pMsg) override;

private:
    /**
     * Return to `multi_load_freq` on retry, or on continue with the slot skipped.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress PAL: 0x001b4048
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};

#endif

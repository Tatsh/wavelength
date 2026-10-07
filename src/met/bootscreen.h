#pragma once

#include "met/freqscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The dialog with which the host confirms removing a guest from the launchpad.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x88 bytes and its vtable
 * is at `0x003cd868`. The metagame registers the class for the screen type `boot_screen`. The
 * dialog has the buttons `yes` and `no`, and both go back to `fn_h_lpad`.
 */
class BootScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no guest.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001880f8
     * @ghidraAddress PAL: 0x0018c338
     */
    explicit BootScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `boot_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035c060
     * @ghidraAddress PAL: 0x003c9bb8
     */
    static UIScreen *New(DataArray *pData) {
        return new BootScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00188200
     * @ghidraAddress PAL: 0x0018c440
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry and put the name of the guest into the question of the dialog.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00188160
     * @ghidraAddress PAL: 0x0018c3a0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Remove the guest for `yes`, and go back to the launchpad for `yes` and `no`.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00188268
     * @ghidraAddress PAL: 0x0018c4a8
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    String mPlayerName; /*!< The name of the guest, set by the screen that opens this one. */
    int mPlayer;        /*!< The guest, set by the screen that opens this one. */
};

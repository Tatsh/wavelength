#pragma once

#include "met/freqscreen.h"
#include "msg/shareremixprogressmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The dialog that shows the progress of sending the remix of an online game to the guests.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x84 bytes and its vtable
 * is at `0x003cd928`. The metagame registers the class for the screen type `share_remix_screen`.
 * When the exchange ends while the screen still enters, the next screen waits in mPendingScreen.
 */
class ShareRemixScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no next screen.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00188340
     * @ghidraAddress PAL: 0x0018c580
     */
    explicit ShareRemixScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `share_remix_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035be68
     * @ghidraAddress PAL: 0x003c99c0
     */
    static UIScreen *New(DataArray *pData) {
        return new ShareRemixScreen(pData);
    }

    /**
     * Route the end of the entry and the progress, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001883b8
     * @ghidraAddress PAL: 0x0018c5f8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show the progress, or go on once the exchange has finished.
     *
     * A guest that received the remix goes to the save screen, and every other console goes back
     * to its launchpad. Without a session, the pending network screen of the metagame shows, or
     * the lost session dialog.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00188448
     * @ghidraAddress PAL: 0x0018c688
     */
    bool HandleProgress(ShareRemixProgressMsg *pMsg);

    /**
     * Go to mPendingScreen, if one waits.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001887b0
     * @ghidraAddress PAL: 0x0018c9f0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    String mPendingScreen; /*!< The screen to go to once the entry finishes, or empty. */
};

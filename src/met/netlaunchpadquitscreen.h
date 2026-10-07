#pragma once

#include "met/freqscreen.h"
#include "msg/launchpadabortedmsg.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The dialog shown while this console exits an online session for the lobby.
 *
 * The RTTI records the class as deriving from FreqScreen, and its vtable is at `0x003cdb08`. The
 * metagame registers the class for the screen type `net_launchpad_quit_screen`.
 */
class NetLaunchpadQuitScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00186878
     * @ghidraAddress PAL: 0x0018aab8
     */
    explicit NetLaunchpadQuitScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `net_launchpad_quit_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035b9a8
     * @ghidraAddress PAL: 0x003c9500
     */
    static UIScreen *New(DataArray *pData) {
        return new NetLaunchpadQuitScreen(pData);
    }

    /**
     * Route the end of the session and the end of the entry, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001868b0
     * @ghidraAddress PAL: 0x0018aaf0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035b9e8
     * @ghidraAddress PAL: 0x003c9540
     */
    const char *Title() override {
        return "";
    }

    /**
     * Exit the session once the screen has entered, or go back to the lobby without one.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00186940
     * @ghidraAddress PAL: 0x0018ab80
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Go back to the lobby.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001869c0
     * @ghidraAddress PAL: 0x0018ac00
     */
    bool HandleAborted(LaunchpadAbortedMsg *pMsg);
};

#pragma once

#include "met/freqscreen.h"
#include "msg/hostlaunchpadsuccessmsg.h"
#include "msg/joypadinputmsg.h"
#include "msg/launchpadabortedmsg.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The dialog shown while this console tries to host an online session.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x74 bytes and its vtable
 * is at `0x003cdc40`. The metagame registers the class for the screen type
 * `net_host_attempt_screen`. The triangle button cancels the attempt.
 */
class NetHostAttemptScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00183d90
     * @ghidraAddress PAL: 0x00187fd0
     */
    explicit NetHostAttemptScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `net_host_attempt_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035b6a0
     * @ghidraAddress PAL: 0x003c91f8
     */
    static UIScreen *New(DataArray *pData) {
        return new NetHostAttemptScreen(pData);
    }

    /**
     * Route the messages of the session, the end of the entry, and the controller buttons, and
     * pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00183dc8
     * @ghidraAddress PAL: 0x00188008
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035b6e0
     * @ghidraAddress PAL: 0x003c9238
     */
    const char *Title() override {
        return "";
    }

    /**
     * Start hosting once the screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00183e98
     * @ghidraAddress PAL: 0x001880d8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Clear the chat of the host launchpad, and go to it unless the player cancelled.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00183ee8
     * @ghidraAddress PAL: 0x00188128
     */
    bool HandleSuccess(HostLaunchpadSuccessMsg *pMsg);

    /**
     * Go back to the hosting menu after a cancel, or to the hosting error otherwise.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00183f90
     * @ghidraAddress PAL: 0x001881d0
     */
    bool HandleAborted(LaunchpadAbortedMsg *pMsg);

    /**
     * Cancel the attempt when the triangle button goes down.
     *
     * With a session, the dialog hides its triangle prompt and shows `cancel_host_attempt` until
     * the session ends. Without one, the screen goes back to the hosting menu.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00183fe0
     * @ghidraAddress PAL: 0x00188220
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mCancelling; /*!< Non-zero once the player cancelled the attempt. */
};

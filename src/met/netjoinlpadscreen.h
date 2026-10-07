#pragma once

#include "met/freqscreen.h"
#include "msg/joinlaunchpadsuccessmsg.h"
#include "msg/joypadinputmsg.h"
#include "msg/launchpadabortedmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The dialog shown while this console tries to join an online session.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003cda48`. The metagame registers the class for the screen type
 * `net_join_lpad_screen`. The triangle button cancels the attempt, and a failure shows the
 * `lpad_error` dialog before going back to the screen the attempt started from.
 */
class NetJoinLPadScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no session chosen.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00186c50
     * @ghidraAddress PAL: 0x0018ae90
     */
    explicit NetJoinLPadScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `net_join_lpad_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035bb88
     * @ghidraAddress PAL: 0x003c96e0
     */
    static UIScreen *New(DataArray *pData) {
        return new NetJoinLPadScreen(pData);
    }

    /**
     * Show the `lpad_error` dialog with the lost session message, unless an error dialog already
     * shows or is about to.
     *
     * @ghidraAddress NTSC-U/C: 0x00187258
     * @ghidraAddress PAL: 0x0018b498
     */
    static void ShowLaunchpadLost();

    /**
     * Route the messages of the session, the end of the entry, and the controller buttons, and
     * pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00186cd8
     * @ghidraAddress PAL: 0x0018af18
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035bbc8
     * @ghidraAddress PAL: 0x003c9720
     */
    const char *Title() override {
        return "";
    }

    /**
     * Choose the session to join.
     *
     * @param nLaunchpadId The first identifier of the session.
     * @param nLaunchpadWorld The second identifier of the session.
     * @ghidraAddress NTSC-U/C: 0x00186cc8
     * @ghidraAddress PAL: 0x0018af08
     */
    void SetLaunchpad(int nLaunchpadId, int nLaunchpadWorld) {
        mLaunchpadWorld = nLaunchpadWorld;
        mLaunchpadId = nLaunchpadId;
    }

    /**
     * Start joining once the screen has entered, recording the screen it entered from.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00186da8
     * @ghidraAddress PAL: 0x0018afe8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Take the settings of the game, clear the chat of the launchpad, and go to it, unless the
     * player cancelled.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00186e20
     * @ghidraAddress PAL: 0x0018b060
     */
    bool HandleSuccess(JoinLaunchpadSuccessMsg *pMsg);

    /**
     * Go back after a cancel, or show the error of the failure.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00186ed0
     * @ghidraAddress PAL: 0x0018b110
     */
    bool HandleAborted(LaunchpadAbortedMsg *pMsg);

    /**
     * Cancel the attempt when the triangle button goes down.
     *
     * With a session, the dialog hides its triangle prompt and shows `cancel_join_attempt` until
     * the session ends. Without one, the screen goes back.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00187060
     * @ghidraAddress PAL: 0x0018b2a0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mLaunchpadId;     /*!< The first identifier of the session, or -1. */
    int mLaunchpadWorld;  /*!< The second identifier of the session, or -1. */
    String mReturnScreen; /*!< The screen the attempt started from. */
    int mCancelling;      /*!< Non-zero once the player cancelled the attempt. */
};

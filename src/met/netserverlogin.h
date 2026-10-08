#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "msg/loginresultmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that logs the local player in to the lobby server and reports the result.
 *
 * The RTTI records the class as deriving from FreqScreen. NetLoginScreen sets the password and the
 * choice of saving it before it changes to this screen.
 */
class NetServerLogin : public FreqScreen {
public:
    /**
     * Construct a screen with an empty password.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017b948
     * @ghidraAddress PAL: 0x0017f5a0
     */
    explicit NetServerLogin(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003598e0
     * @ghidraAddress PAL: 0x003c6c48
     */
    ~NetServerLogin() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x00359988
     * @ghidraAddress PAL: 0x003c6cf0
     */
    static UIScreen *New(DataArray *pData) {
        return new NetServerLogin(pData);
    }

    /**
     * Route the end of the entry, the login result, and the controller messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017b990
     * @ghidraAddress PAL: 0x0017f5e8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x003599c8
     * @ghidraAddress PAL: 0x003c6d30
     */
    const char *Title() override {
        return "";
    }

    int mLocation;     /*!< The identifier of the lobby location NetServerSelScreen chose. */
    String mPassword;  /*!< The password to log in with. */
    int mSaveChanged;  /*!< Non-zero when the choice of saving the password changed. */
    int mSavePassword; /*!< Whether the password is saved in the profile after the login. */

private:
    /**
     * Start the login once the screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017ba40
     * @ghidraAddress PAL: 0x0017f698
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Take the account, the news, and the licence agreement of a login, and go on to the next
     * screen, or show the error.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017baa8
     * @ghidraAddress PAL: 0x0017f700
     */
    bool HandleLoginResult(LoginResultMsg *pMsg);

    /**
     * Exit the lobby when the triangle button is pressed.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True while a transition runs, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x0017bdf0
     * @ghidraAddress PAL: 0x0017fa48
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};

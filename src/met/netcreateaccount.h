#pragma once

#include "met/freqscreen.h"
#include "msg/createaccountresultmsg.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that creates the online account and goes on to the login, or shows the error.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetCreateAccount : public FreqScreen {
public:
    /**
     * Construct a screen with an empty password.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017b3a8
     * @ghidraAddress PAL: 0x0017f000
     */
    explicit NetCreateAccount(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003595c8
     * @ghidraAddress PAL: 0x003c6930
     */
    ~NetCreateAccount() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x00359670
     */
    static UIScreen *New(DataArray *pData) {
        return new NetCreateAccount(pData);
    }

    /**
     * Route the end of the entry, the result of the creation, and the controller messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017b3f0
     * @ghidraAddress PAL: 0x0017f048
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x003596b0
     */
    const char *Title() override {
        return "";
    }

    String mPassword; /*!< The password of the new account. */

    /**
     * Whether the login saves the password.
     *
     * The constructor does not set it, and the screen only reads it.
     */
    int mSavePassword;

private:
    /**
     * Create the account once the screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017b4a0
     * @ghidraAddress PAL: 0x0017f0f8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Lock the name and log in, or show the error of the creation.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017b500
     * @ghidraAddress PAL: 0x0017f158
     */
    bool HandleCreateAccountResult(CreateAccountResultMsg *pMsg);

    /**
     * Exit the lobby when the triangle button is pressed.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True while a transition runs, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x0017b630
     * @ghidraAddress PAL: 0x0017f288
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};

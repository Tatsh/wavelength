#pragma once

#include "met/keyboarduser.h"
#include "met/netpasswordscreen.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uiscreen.h"
#include "ui/uitextentrycompletemsg.h"

/**
 * Screen where the player enters the password of the online account and logs in.
 *
 * The RTTI records the class as deriving from NetPasswordScreen and from KeyboardUser. A profile
 * that has never logged in may only create an account. The on-screen keyboard types the password.
 */
class NetLoginScreen : public NetPasswordScreen, public KeyboardUser {
public:
    /**
     * Construct a screen with no typed password.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x001786c8
     * @ghidraAddress PAL: 0x0017c2d8
     */
    explicit NetLoginScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00359160
     * @ghidraAddress PAL: 0x003c64c8
     */
    ~NetLoginScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetLoginScreen(pData);
    }

    /**
     * Route the controller, the end of the entry, and the focus messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00178c10
     * @ghidraAddress PAL: 0x0017c830
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, and fill the name and the password from the profile or from the keyboard.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x00178738
     * @ghidraAddress PAL: 0x0017c348
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Take the password the keyboard typed.
     *
     * @param pszText The password.
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x00179008
     * @ghidraAddress PAL: 0x0017cc28
     */
    int ReceiveKeyboardText(const char *pszText) override;

private:
    /**
     * Pass the password and the choice of saving it to NetServerLogin and change to it.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001789b8
     * @ghidraAddress PAL: 0x0017c5d0
     */
    void Login();

    /**
     * Open the on-screen keyboard for the password.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00178ad8
     * @ghidraAddress PAL: 0x0017c6f0
     */
    void OpenKeyboard();

    /**
     * Log in or open the keyboard with the cross button on the password, and move the focus off the
     * `save` button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True when the focus moved or a transition runs, otherwise the result of
     * FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x00178cc0
     * @ghidraAddress PAL: 0x0017c8e0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Start or stop editing the password as the focus moves on or off it.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00178ee8
     * @ghidraAddress PAL: 0x0017cb08
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Log in when an entry of text ends.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x00179028
     * @ghidraAddress PAL: 0x0017cc48
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);

    String mTypedPassword; /*!< The password the keyboard typed, or empty. */
};

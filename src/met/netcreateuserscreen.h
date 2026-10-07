#pragma once

#include "met/keyboarduser.h"
#include "met/netpasswordscreen.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"
#include "ui/uitextentrycompletemsg.h"
#include "ui/uitextentryinvalidmsg.h"

/**
 * Screen where the player chooses the name and the password of a new online account.
 *
 * The RTTI records the class as deriving from NetPasswordScreen and from KeyboardUser. The `login`
 * button checks the entries and goes on to NetCreateAccount. While the on-screen keyboard types an
 * entry, the screen retains the texts of the entries.
 */
class NetCreateUserScreen : public NetPasswordScreen, public KeyboardUser {
public:
    /**
     * Construct a screen with empty entries.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x00179058
     * @ghidraAddress PAL: 0x0017cc78
     */
    explicit NetCreateUserScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003592b8
     * @ghidraAddress PAL: 0x003c6620
     */
    ~NetCreateUserScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetCreateUserScreen(pData);
    }

    /**
     * Route the controller, focus, choice, and text entry messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00179b08
     * @ghidraAddress PAL: 0x0017d748
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, fill the entries back from before the keyboard, and save the password by default.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x00179128
     * @ghidraAddress PAL: 0x0017cd48
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Take the text the keyboard typed.
     *
     * @param pszText The text.
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x0017a2f0
     * @ghidraAddress PAL: 0x0017df30
     */
    int ReceiveKeyboardText(const char *pszText) override;

private:
    /**
     * Check that the passwords agree and that the name is valid, then pass the account to
     * NetCreateAccount and change to it.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001794e8
     * @ghidraAddress PAL: 0x0017d110
     */
    void Submit();

    /**
     * Retain the entries and open the on-screen keyboard for one of them.
     *
     * The name is inferred.
     *
     * @param pszEntry The name of the entry.
     * @ghidraAddress NTSC-U/C: 0x00179800
     * @ghidraAddress PAL: 0x0017d430
     */
    void OpenKeyboard(const char *pszEntry);

    /**
     * Move from one entry to the next with the cross button, or open the keyboard.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True when the focus moved or a transition runs, otherwise the result of
     * FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x00179bf8
     * @ghidraAddress PAL: 0x0017d838
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Light the prefix of the entry that has the focus and edit it.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00179fb0
     * @ghidraAddress PAL: 0x0017dbf0
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Submit when the `login` button is chosen.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0017a140
     * @ghidraAddress PAL: 0x0017dd80
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Move on to the next entry when the entry of text ends.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017a1a0
     * @ghidraAddress PAL: 0x0017dde0
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);

    /**
     * Play the error sound when the name refuses a character.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017a2b0
     * @ghidraAddress PAL: 0x0017def0
     */
    bool HandleTextEntryInvalid(UITextEntryInvalidMsg *pMsg);

    String mKeyboardText;  /*!< The text the keyboard typed, or empty. */
    String mKeyboardEntry; /*!< The entry the keyboard types, or empty. */
    String mUserName;      /*!< The name entry, retained while the keyboard shows. */
    String mPassword;      /*!< The password entry, retained while the keyboard shows. */
    String mConfirm;       /*!< The confirmation entry, retained while the keyboard shows. */
};

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

/**
 * Screen where the player enters the old password and a new one.
 *
 * The RTTI records the class as deriving from NetPasswordScreen and from KeyboardUser. The `done`
 * button checks the entries and goes on to NetChangePassword. While the on-screen keyboard types an
 * entry, the screen retains the texts of the entries.
 */
class NetChangePasswordScreen : public NetPasswordScreen, public KeyboardUser {
public:
    /**
     * Construct a screen with empty entries.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017a310
     * @ghidraAddress PAL: 0x0017df50
     */
    explicit NetChangePasswordScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00359440
     * @ghidraAddress PAL: 0x003c67a8
     */
    ~NetChangePasswordScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetChangePasswordScreen(pData);
    }

    /**
     * Route the controller, focus, choice, and text entry messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017ac30
     * @ghidraAddress PAL: 0x0017e888
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter and fill the entries back from before the keyboard.
     *
     * Unless the keyboard is the screen that exits, the `old` entry takes the focus.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017a3e0
     * @ghidraAddress PAL: 0x0017e020
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Take the text the keyboard typed.
     *
     * @param pszText The text.
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x0017b388
     * @ghidraAddress PAL: 0x0017efe0
     */
    int ReceiveKeyboardText(const char *pszText) override;

private:
    /**
     * Check that the new passwords agree, then pass the passwords to NetChangePassword and change
     * to it.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0017a788
     * @ghidraAddress PAL: 0x0017e3d8
     */
    void Submit();

    /**
     * Retain the entries and open the on-screen keyboard for one of them.
     *
     * The name is inferred.
     *
     * @param pszEntry The name of the entry.
     * @ghidraAddress NTSC-U/C: 0x0017a958
     * @ghidraAddress PAL: 0x0017e5a8
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
     * @ghidraAddress NTSC-U/C: 0x0017ad00
     * @ghidraAddress PAL: 0x0017e958
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Light the prefix of the entry that has the focus and edit it.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017b0b8
     * @ghidraAddress PAL: 0x0017ed10
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Submit when the `done` button is chosen.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0017b218
     * @ghidraAddress PAL: 0x0017ee70
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Move on to the next entry when the entry of text ends.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017b278
     * @ghidraAddress PAL: 0x0017eed0
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);

    String mKeyboardText;  /*!< The text the keyboard typed, or empty. */
    String mKeyboardEntry; /*!< The entry the keyboard types, or empty. */
    String mOld;           /*!< The old password entry, retained while the keyboard shows. */
    String mNew;           /*!< The new password entry, retained while the keyboard shows. */
    String mConfirm;       /*!< The confirmation entry, retained while the keyboard shows. */
};

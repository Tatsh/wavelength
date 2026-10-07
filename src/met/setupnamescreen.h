#pragma once

#include "met/freqscreen.h"
#include "met/keyboarduser.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uitextentry.h"
#include "ui/uitextentrycompletemsg.h"

/**
 * Screen where the player types a name on the `name` entry of its panel, for a subclass to submit.
 *
 * The RTTI records the class as deriving from FreqScreen and from KeyboardUser.
 */
class SetupNameScreen : public FreqScreen, public KeyboardUser {
public:
    /**
     * Construct a screen with no text.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017d0c8
     * @ghidraAddress PAL: 0x00180d20
     */
    explicit SetupNameScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035a638
     * @ghidraAddress PAL: 0x003c8188
     */
    ~SetupNameScreen() override {
    }

    /**
     * Route the controller and the end of the entry.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017d240
     * @ghidraAddress PAL: 0x00180e98
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, fill the `name` entry with the text, and edit it.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017d138
     * @ghidraAddress PAL: 0x00180d90
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Take the text the keyboard typed.
     *
     * @param pszText The text.
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x0017d220
     * @ghidraAddress PAL: 0x00180e78
     */
    int ReceiveKeyboardText(const char *pszText) override;

    /**
     * Set the text the `name` entry shows on the next entry of the screen.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x0035a718
     * @ghidraAddress PAL: 0x003c8268
     */
    virtual void SetText(const char *pszText) {
        mText = pszText;
    }

    /**
     * Submit the name.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0035a738
     * @ghidraAddress PAL: 0x003c8288
     */
    virtual void Submit() {
    }

protected:
    String mText;        /*!< The text for the `name` entry, or empty. */
    UITextEntry *mEntry; /*!< The `name` entry, from the last entry of the screen. */

private:
    /**
     * Open the keyboard with the circle button, or submit with the cross button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True while a transition runs, otherwise the result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0017d2d0
     * @ghidraAddress PAL: 0x00180f28
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Submit when the entry of text ends.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x0017d498
     * @ghidraAddress PAL: 0x001810e8
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);
};

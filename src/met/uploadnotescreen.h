#pragma once

#include "met/freqscreen.h"
#include "met/keyboarduser.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"
#include "ui/uitextentrycompletemsg.h"

/**
 * Screen where the player writes the note uploaded with a remix.
 *
 * The RTTI records the class as deriving from FreqScreen and from KeyboardUser. The `upload`
 * button passes the note to NetDoUploadScreen.
 */
class UploadNoteScreen : public FreqScreen, public KeyboardUser {
public:
    /**
     * Construct a screen with no note.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017eae8
     */
    explicit UploadNoteScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035af88
     * @ghidraAddress PAL: 0x003c8ad8
     */
    ~UploadNoteScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x0035b068
     */
    static UIScreen *New(DataArray *pData) {
        return new UploadNoteScreen(pData);
    }

    /**
     * Route the controller, choice, focus, and text entry messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017ec28
     * @ghidraAddress PAL: 0x00182910
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, give the focus to the `note` entry, and fill it with the text the keyboard typed.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017eb58
     * @ghidraAddress PAL: 0x00182840
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Take the text the keyboard typed.
     *
     * @param pszText The text.
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x0017ec08
     * @ghidraAddress PAL: 0x001828f0
     */
    int ReceiveKeyboardText(const char *pszText) override;

private:
    /**
     * Open the keyboard for the note with the circle button, or move to `upload` with the cross
     * button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True when a transition runs or the focus moved, otherwise the result of
     * FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0017ecf8
     * @ghidraAddress PAL: 0x001829e0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Pass the note to NetDoUploadScreen when `upload` is chosen.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0017ef08
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Edit the note while it has the focus.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017f030
     * @ghidraAddress PAL: 0x00182da8
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Move the focus to `upload` when the entry of the note ends.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x0017f108
     * @ghidraAddress PAL: 0x00182e80
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);

    String mKeyboardText; /*!< The text the keyboard typed, or empty. */
};

#pragma once

#include "met/keyboarduser.h"
#include "met/needsdialogscreen.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uitextentrycompletemsg.h"
#include "ui/uitextentryinvalidmsg.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The screen after a remix that shows its details and takes the name it is saved under.
 *
 * The RTTI records the class as deriving from NeedsDialogScreen and KeyboardUser, with KeyboardUser
 * at `+0x70`. The object is 0x9c bytes and its vtables are at `0x003ce6a8` and `0x003ce688`. The
 * metagame registers the class for the screen type `setup_remix_save_screen`. The description
 * provides `band_pic_panel`, the panel of the details. The name is the text entry `title`. The
 * cross button saves, the circle button opens the keyboard, and the square button queries whether
 * to discard the remix.
 */
class SetupRemixSaveScreen : public NeedsDialogScreen, public KeyboardUser {
public:
    /**
     * Construct the screen from its script description, for the first player.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00189fe8
     * @ghidraAddress PAL: 0x00190580
     */
    explicit SetupRemixSaveScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `setup_remix_save_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035c658
     * @ghidraAddress PAL: 0x003ca730
     */
    static UIScreen *New(DataArray *pData) {
        return new SetupRemixSaveScreen(pData);
    }

    /**
     * Route the messages of the screen, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018a978
     * @ghidraAddress PAL: 0x00190f40
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry and show the details of the remix of the game database.
     *
     * The name comes from the keyboard after it, or else from the remix or the song. The date of
     * the remix becomes the current date.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018a088
     * @ghidraAddress PAL: 0x00190620
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Check the name, and show the error of an empty name or of spaces around it, or else save the
     * remix under the name with `save_remix`.
     *
     * @ghidraAddress NTSC-U/C: 0x0018a7a8
     */
    void Proceed() override;

    /**
     * Store the text typed on the keyboard for the next entry.
     *
     * @param pszText The text.
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x0018a788
     * @ghidraAddress PAL: 0x00190cd0
     */
    int ReceiveKeyboardText(const char *pszText) override;

    /**
     * Handle the buttons of the screen's player while the name has the focus.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise the result of
     * FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0018aa48
     * @ghidraAddress PAL: 0x00191010
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Show the panels again when the discard question closes.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleTransitionComplete().
     * @ghidraAddress NTSC-U/C: 0x0018acb0
     * @ghidraAddress PAL: 0x00191280
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Save the remix when a name was typed into the entry.
     *
     * @param pMsg The message.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x0018ada8
     * @ghidraAddress PAL: 0x00191378
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);

    /**
     * Play the refusal sound for a character the name entry refused.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0018ade0
     * @ghidraAddress PAL: 0x001913b0
     */
    bool HandleTextEntryInvalid(UITextEntryInvalidMsg *pMsg);

    int mReserved74[3]; // +0x74, not yet recovered.
    const char *mPanel; /*!< The `band_pic_panel` entry of the description. */
    int mPlayer;        /*!< The player who saves, from 1. */
    String mTypedTitle; /*!< The text the keyboard returned, until the next entry takes it. */
};

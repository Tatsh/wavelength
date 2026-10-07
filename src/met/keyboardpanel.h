#pragma once

#include "met/freqpanel.h"
#include "met/keyboardrequest.h"
#include "msg/joypadinputmsg.h"
#include "msg/keyboardkeymsg.h"
#include "rnd/matanim.h"
#include "script/dataarray.h"
#include "ui/uibutton.h"
#include "ui/uicomponent.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uitextentry.h"
#include "ui/uitextentryinvalidmsg.h"

/**
 * The front-end keyboard, for typing a name or another text for the screen that opened it.
 *
 * The RTTI records the class as deriving from FreqPanel. The object is 0x140 bytes and its vtable
 * is at `0x003d0370`. The metagame registers the class for the panel type `kb_panel`, and the
 * front-end description's `keyboard` panel of the `kb_screen` screen is one. The entry is the
 * text entry `01`. Each key is a KeyboardKey whose text it types, and the keys `but_caps`,
 * `but_lshift`, `but_rshift`, `but_back`, `but_larrow`, `but_rarrow`, `but_tab`, `but_space`,
 * `but_del`, `but_enter`, and the function keys `but_f1` to `but_f12` act instead. The
 * controller types as well. Circle is enter, square a space, L1 and R1 move the caret, L2 is
 * backspace, R2 shifts the next key, and triangle returns without the text. A keyboard attached
 * to the console types directly.
 */
class KeyboardPanel : public FreqPanel {
public:
    /** The shift states of the keys, the values of mShiftMode. */
    enum ShiftMode {
        kShiftNone = 0, /*!< The keys type their plain characters. */
        kShiftOnce = 1, /*!< The next key types its shifted character. */
        kShiftCaps = 2, /*!< The keys type their capital characters until caps is chosen again. */
    };

    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x001a8b08
     * @ghidraAddress PAL: 0x001b07f8
     */
    KeyboardPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003625f8
     * @ghidraAddress PAL: 0x003d0b10
     */
    ~KeyboardPanel() override;

    /**
     * Create a panel from its script description.
     *
     * The metagame registers the routine for the entry type `kb_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003625a8
     * @ghidraAddress PAL: 0x003d0ac0
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new KeyboardPanel(pData, pszDir);
    }

    /**
     * Route keys about to be chosen, controller buttons, keys of an attached keyboard, and
     * refused characters, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a9c68
     * @ghidraAddress PAL: 0x001b1968
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry and prepare the text entry from the request.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a8b58
     * @ghidraAddress PAL: 0x001b0850
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Start the exit and forget the keys and the animation.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a8e80
     * @ghidraAddress PAL: 0x001b0b78
     */
    void Exit(bool bForce, float fTime) override;

    /**
     * Advance the panel and the animation of the selected key.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a8f08
     * @ghidraAddress PAL: 0x001b0c00
     */
    void Poll(float fTime) override;

    /**
     * Play the `KEYBOARD_LEFT_UP` cue and move the focus to a key.
     *
     * @param pComponent The key.
     * @param nButton The controller button that moved the focus, or kPadNone.
     * @ghidraAddress NTSC-U/C: 0x001a8eb8
     * @ghidraAddress PAL: 0x001b0bb0
     */
    void SetFocus(UIComponent *pComponent, int nButton) override;

    /**
     * Take the request of the screen that opens the keyboard.
     *
     * @param request The request.
     * @ghidraAddress NTSC-U/C: 0x001a8f50
     * @ghidraAddress PAL: 0x001b0c48
     */
    void SetRequest(const KeyboardRequest &request);

    /**
     * Act on the key about to be chosen.
     *
     * @param pMsg The message.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x001a8fd8
     * @ghidraAddress PAL: 0x001b0cd8
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Flash the key a controller button stands for.
     *
     * @param pszKey The key.
     * @ghidraAddress NTSC-U/C: 0x001a91d8
     * @ghidraAddress PAL: 0x001b0ed8
     */
    void FlashKey(const char *pszKey);

    /**
     * Act on a button of the controller that types.
     *
     * @param pMsg The message.
     * @return True for another controller or a button that typed, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x001a9238
     * @ghidraAddress PAL: 0x001b0f38
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Act on a key of a keyboard attached to the console.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001a93a0
     * @ghidraAddress PAL: 0x001b10a0
     */
    bool HandleKey(KeyboardKeyMsg *pMsg);

    /**
     * Change the shift state, restyling the shift keys and relabelling every key.
     *
     * A key `<name>` shows the text `<name>`, `<name>_shift`, or `<name>_caps` of the locale.
     * Choosing the current shift state again returns to kShiftNone.
     *
     * @param nMode One of ShiftMode.
     * @ghidraAddress NTSC-U/C: 0x001a9628
     * @ghidraAddress PAL: 0x001b1328
     */
    void SetShiftMode(int nMode);

    /**
     * Erase the character before the caret.
     *
     * @ghidraAddress NTSC-U/C: 0x001a9928
     * @ghidraAddress PAL: 0x001b1628
     */
    void Backspace();

    /**
     * Move the caret left.
     *
     * @ghidraAddress NTSC-U/C: 0x001a9950
     * @ghidraAddress PAL: 0x001b1650
     */
    void MoveLeft();

    /**
     * Move the caret right.
     *
     * @ghidraAddress NTSC-U/C: 0x001a9978
     * @ghidraAddress PAL: 0x001b1678
     */
    void MoveRight();

    /**
     * Type a tab.
     *
     * @ghidraAddress NTSC-U/C: 0x001a99a0
     * @ghidraAddress PAL: 0x001b16a0
     */
    void TypeTab();

    /**
     * Type a space.
     *
     * @ghidraAddress NTSC-U/C: 0x001a99c0
     * @ghidraAddress PAL: 0x001b16c0
     */
    void TypeSpace();

    /**
     * Erase the character after the caret.
     *
     * @ghidraAddress NTSC-U/C: 0x001a99e0
     * @ghidraAddress PAL: 0x001b16e0
     */
    void Delete();

    /**
     * Hand the text to the user and return to the screen that opened the keyboard, unless the
     * user moved to another screen.
     *
     * @ghidraAddress NTSC-U/C: 0x001a9a08
     * @ghidraAddress PAL: 0x001b1708
     */
    void Commit();

    /**
     * Replace the text with the text `fkey_<key>` of the locale, or go to `kb_fkey_error_screen`
     * when the request disallows the function keys.
     *
     * @param pszKey The function key, such as `f1`.
     * @ghidraAddress NTSC-U/C: 0x001a9ab0
     * @ghidraAddress PAL: 0x001b17b0
     */
    void TypeFunctionKey(const char *pszKey);

    /**
     * Type a character, replacing the starting text with the first one typed, and end a shift of
     * one key.
     *
     * @param ch The character.
     * @ghidraAddress NTSC-U/C: 0x001a9b50
     * @ghidraAddress PAL: 0x001b1850
     */
    void TypeChar(char ch);

    /**
     * Play the `WRONG` cue when the text is full, or go to `kb_badkey_error_screen` for a
     * character the entry does not accept.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001a9bd0
     * @ghidraAddress PAL: 0x001b18d0
     */
    bool HandleInvalid(UITextEntryInvalidMsg *pMsg);

    KeyboardRequest mRequest;    /*!< The request of the screen that opened the keyboard. +0xe0 */
    int mShiftMode;              /*!< One of ShiftMode. +0x118 */
    int mReplaceText;            /*!< Non-zero while the next character replaces the text. */
    UITextEntry *mEntry;         /*!< The text entry `01`. */
    UIButton *mCapsButton;       /*!< The key `but_caps`, while the panel shows. */
    UIButton *mLeftShiftButton;  /*!< The key `but_lshift`, while the panel shows. */
    UIButton *mRightShiftButton; /*!< The key `but_rshift`, while the panel shows. */
    Rnd::MatAnim *mSelectAnim;   /*!< The animation `keyboard_sel.mnm`, or null. +0x130 */
};

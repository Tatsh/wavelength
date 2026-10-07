#pragma once

#include "game/avatarpartset.h"
#include "met/freqmakerundoscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uibutton.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * The custom screen of the Freq maker, with a button for each part the player can change.
 *
 * The RTTI records the class as deriving from FreqMakerUndoScreen. The object is 0xc0 bytes and
 * its vtable is at `0x003cfe30`. The metagame registers the class for the screen type
 * `f_maker_custom_screen`, and the front-end description's `f_maker_custom` screen is one. Its
 * `f_maker_c` panel leads from each part to `f_maker_part`, from `emblems` to `f_maker_emblem`,
 * and from `done` to `f_maker`. The screen copies the Freq when it first enters, to restore it
 * when the player abandons the changes.
 */
class FreqMakerCustomScreen : public FreqMakerUndoScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00360ed8
     * @ghidraAddress PAL: 0x003cf3f0
     */
    explicit FreqMakerCustomScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00360db8
     * @ghidraAddress PAL: 0x003cf2d0
     */
    ~FreqMakerCustomScreen() override;

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `f_maker_custom_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00360e98
     * @ghidraAddress PAL: 0x003cf3b0
     */
    static UIScreen *New(DataArray *pData) {
        return new FreqMakerCustomScreen(pData);
    }

    /**
     * Route controller buttons and buttons about to be chosen, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a0aa8
     * @ghidraAddress PAL: 0x001a8788
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Copy the Freq unless a copy is pending, and start the entry.
     *
     * The gear and emblem buttons are disabled when the player has unlocked no choice for them,
     * and `done` is disabled while a needed part is missing. Entering from `f_maker` forgets
     * earlier changes.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a0428
     * @ghidraAddress PAL: 0x001a8108
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report the localised title, `f_maker_c_TITLE`.
     *
     * @return The title.
     * @ghidraAddress NTSC-U/C: 0x001a0710
     * @ghidraAddress PAL: 0x001a83f0
     */
    const char *Title() override;

    /**
     * Restore the Freq from the copy the entry made.
     *
     * @ghidraAddress NTSC-U/C: 0x001a07e0
     * @ghidraAddress PAL: 0x001a84c0
     */
    void Undo() override;

    /**
     * Disable `done` while a needed part of the Freq is missing, and enable it otherwise.
     *
     * @ghidraAddress NTSC-U/C: 0x001a0740
     * @ghidraAddress PAL: 0x001a8420
     */
    void UpdateDone();

    /**
     * Return to `f_maker` on the triangle button, through `f_maker_lose_custom_changes` when the
     * Freq changed.
     *
     * @param pMsg The message.
     * @return True while the screen moves, otherwise the result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x001a08b0
     * @ghidraAddress PAL: 0x001a8590
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Accept the changes on `done`, remove the parts on `clear`, and note a change on the other
     * buttons chosen with the cross button.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x001a09f0
     * @ghidraAddress PAL: 0x001a86d0
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    AvatarPartSet mSaved;  /*!< The Freq as it was when the screen first entered. +0x70 */
    int mSavedValid;       /*!< Non-zero while mSaved is the Freq to restore. +0xac */
    int mChanged;          /*!< Non-zero once the Freq changed on the screen. */
    UIButton *mDoneButton; /*!< The `done` button. */
};

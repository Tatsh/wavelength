#pragma once

#include "memcard/memcarduser.h"
#include "met/overwritesavescreen.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that saves the Freq of the first player to the memory card, such as `pre_f_maker_save`.
 *
 * The RTTI records the class as deriving from OverwriteSaveScreen and from MemcardUser at
 * `+0xb0`. The object is 0xd0 bytes and its vtables are at `0x003d1168` and, for MemcardUser,
 * `0x003d10d8`. The metagame registers the class for the screen type `save_freq_screen`. Its
 * `save_freq_dlg` MCDialogPanel shows while the save runs. The save starts once the entry ends,
 * under the name of the first player. The description's `overwrite_status` allows replacing a
 * saved Freq, and `isCancel` (on by default) also offers to continue without the save when the
 * memory card fails.
 */
class SaveFreqScreen : public OverwriteSaveScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001abec0
     */
    explicit SaveFreqScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003634d0
     * @ghidraAddress PAL: 0x003d1ac8
     */
    ~SaveFreqScreen() override;

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `save_freq_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00363608
     */
    static UIScreen *New(DataArray *pData) {
        return new SaveFreqScreen(pData);
    }

    /**
     * Route the end of the entry, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ac270
     * @ghidraAddress PAL: 0x001b47a8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry. Once a cheat was entered the Freq is not saved, and the screen moves to
     * `cheat_no_save_screen`. That dialog leads to the done screen.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001abf48
     * @ghidraAddress PAL: 0x001b4248
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x00363648
     */
    const char *Title() override {
        return "";
    }

    /**
     * Start the save once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ac2d8
     * @ghidraAddress PAL: 0x001b4810
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Move on when the save ends. A save leads to the done screen and marks the profile as saved,
     * and a failure opens its dialog.
     *
     * The memory card task reports here through the third virtual routine of MemcardUser.
     *
     * @param nStatus How the save ended: 0 for a save, 1 without a memory card, 2 without enough
     *                space, 3 for an unformatted memory card, 4 for another memory card than
     *                before, 5 when a Freq of the name exists, and 6 when the memory card has no
     *                room for another Freq.
     * @param nSpace The space the save needs, when the memory card is too full.
     * @ghidraAddress NTSC-U/C: 0x001ac010
     */
    void OnFreqSaved(int nStatus, int nSpace);

    int mReservedB4[3]; // +0xb4, not yet recovered.
    int mIsCancel;      /*!< Non-zero when a failure also offers to continue, `isCancel`. +0xc0 */
};

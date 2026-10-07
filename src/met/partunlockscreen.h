#pragma once

#include <vector>

#include "game/avatarpartset.h"
#include "game/unlockableitem.h"
#include "met/freqscreen.h"
#include "met/unlockavatarpanel.h"
#include "met/viewanimplayer.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that lists the Freq parts a song unlocked, seven to a page, and shows an unlocked
 * prefabricated Freq on a page of its own.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0xe0 bytes and its vtable
 * is at `0x003cf450`. The metagame registers the class for the screen type `parts_unlock_screen`.
 * Each page plays the view `s_unlock.view`, and the next page shows `unlock_prefab_time`
 * milliseconds after the previous one. The destructor at `0x0035e928` is compiler-generated and
 * has no declaration here.
 */
class PartUnlockScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, listing no item.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0035ea68
     * @ghidraAddress PAL: 0x003ccb48
     */
    explicit PartUnlockScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `parts_unlock_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035ea28
     * @ghidraAddress PAL: 0x003ccb08
     */
    static UIScreen *New(DataArray *pData) {
        return new PartUnlockScreen(pData);
    }

    /**
     * Route the end of a transition and a controller button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00198230
     * @ghidraAddress PAL: 0x0019f750
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Apply the parts of the Freq as they load, and show the next page once its time has come,
     * or move to `unlock_parts_done` after the last page.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00197848
     * @ghidraAddress PAL: 0x0019ed68
     */
    void Poll(float fTime) override;

    /**
     * Forget the listed items and stop the view.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001977b8
     * @ghidraAddress PAL: 0x0019ecd8
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Find the panel of the Freq and show the first page.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00197660
     * @ghidraAddress PAL: 0x0019eb80
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Queue an item. The kinds 4, 6, and 15 are not listed.
     *
     * Queueing the head gear `fm_hg_halo` also shows the Freq of the first player with a halo.
     *
     * @param item The item.
     * @return Whether the item was queued.
     * @ghidraAddress NTSC-U/C: 0x00197f00
     * @ghidraAddress PAL: 0x0019f420
     */
    bool AddItem(const UnlockableItem &item);

    std::vector<UnlockableItem> mItems; /*!< The queued items. */
    int mIndex;                         /*!< The index in mItems of the first item not shown. */
    ViewAnimPlayer mAnimPlayer;         /*!< The player of `s_unlock.view`. +0x84 */
    AvatarPartSet mPrefab;              /*!< The prefabricated Freq of the page. */
    UnlockAvatarPanel *mPanel;          /*!< The panel `s_unloc_p` that shows the Freq. +0xd0 */
    float mNextMs;                      /*!< The system time the next page shows. */
    float mDelayMs;                     /*!< `unlock_prefab_time`, the time between pages. */
    int mHalo;                          /*!< Whether `fm_hg_halo` is among the items. */

private:
    /**
     * Set the text of `s_unlock_<line>.txt`.
     *
     * @param nLine The number of the text.
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x00197ac8
     * @ghidraAddress PAL: 0x0019efe8
     */
    void SetLine(int nLine, const char *pszText);

    /**
     * Show an item in a row, its kind in one text and its name in the next.
     *
     * @param nRow The row, 0 for the prefabricated Freq.
     * @param nKind The kind of the item, one of UnlockableItem::Kind.
     * @param pszName The localised name, or null to clear the row.
     * @ghidraAddress NTSC-U/C: 0x00197b70
     * @ghidraAddress PAL: 0x0019f090
     */
    void SetLinePair(int nRow, int nKind, const char *pszName);

    /**
     * Show the next page of items.
     *
     * @ghidraAddress NTSC-U/C: 0x00197c18
     * @ghidraAddress PAL: 0x0019f138
     */
    void ShowNextItems();

    /**
     * Report the localised label of a kind of item.
     *
     * @param nKind One of UnlockableItem::Kind.
     * @return The label.
     * @ghidraAddress NTSC-U/C: 0x00197e38
     * @ghidraAddress PAL: 0x0019f358
     */
    const char *PartLabel(int nKind);

    /**
     * Start the view and time the next page.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleTransitionComplete().
     * @ghidraAddress NTSC-U/C: 0x00198148
     * @ghidraAddress PAL: 0x0019f668
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Swallow a button that goes down while the screen enters or exits.
     *
     * @param pMsg The message of the button.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001981f0
     * @ghidraAddress PAL: 0x0019f710
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};

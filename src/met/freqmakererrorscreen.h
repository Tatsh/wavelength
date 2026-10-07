#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * Screen that offers to abandon the changes of a Freq maker screen.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x80 bytes and its vtable
 * is at `0x003cfd10`. The metagame registers the class for the screen type
 * `f_maker_error_screen`, and the front-end description's `f_maker_lose_main_changes` and
 * `f_maker_lose_custom_changes` screens are two. The screen that opens it sets the two screens
 * with SetScreens(). The destructor at `0x00361120` is compiler-generated and has no declaration
 * here.
 */
class FreqMakerErrorScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x003611f8
     * @ghidraAddress PAL: 0x003cf710
     */
    explicit FreqMakerErrorScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `f_maker_error_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003611b8
     * @ghidraAddress PAL: 0x003cf6d0
     */
    static UIScreen *New(DataArray *pData) {
        return new FreqMakerErrorScreen(pData);
    }

    /**
     * Route a button about to be chosen, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a2568
     * @ghidraAddress PAL: 0x001aa248
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Find the screens the buttons lead to.
     *
     * @param pszContinue The screen `continue` leads to.
     * @param pszCancel The Freq maker screen `cancel` returns to. Its changes are abandoned when
     *                  the player continues.
     * @ghidraAddress NTSC-U/C: 0x001a2418
     * @ghidraAddress PAL: 0x001aa0f8
     */
    void SetScreens(const char *pszContinue, const char *pszCancel);

    /**
     * Abandon the changes and continue on `continue`, or return on `cancel`.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x001a2478
     * @ghidraAddress PAL: 0x001aa158
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    UIScreen *mContinueScreen; /*!< The screen `continue` leads to. */
    UIScreen *mCancelScreen;   /*!< The Freq maker screen `cancel` returns to. */
};

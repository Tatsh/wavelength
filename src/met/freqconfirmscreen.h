#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The Freq menu. It confirms the player's Freq, or leads to the Freq maker to edit, create, or
 * load one.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003cee48`. The metagame registers the class for the screen type `confirm_screen`, and
 * the front-end description's `f_confirm` screen is one. Its transition table leads from
 * `confirm` to `solofreq2soloskill`, from `edit` and `create` to `f_maker`, and from `load` to
 * `load_freq`. The destructor at `0x0035d4f8` is compiler-generated and has no declaration here.
 */
class FreqConfirmScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018e368
     * @ghidraAddress PAL: 0x00195328
     */
    explicit FreqConfirmScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `confirm_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d590
     * @ghidraAddress PAL: 0x003cb668
     */
    static UIScreen *New(DataArray *pData) {
        return new FreqConfirmScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018e608
     * @ghidraAddress PAL: 0x00195920
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry and label the buttons with the name of the player's Freq.
     *
     * A player without a Freq of their own, or any player online once a cheat was entered, cannot
     * confirm or edit a Freq. The focus then rests on `load` when the memory card has Freqs, and
     * on `create` otherwise.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018e3a0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Prepare the Freq maker or the load screen for the button chosen with the cross button.
     *
     * Before creating or loading a Freq, a player whose Freq changed since it was saved is
     * offered to save it through the `cur_freq_not_saved` screen. Creating a Freq resets the game
     * to one default player.
     *
     * @param pMsg The message.
     * @return True when the save offer opened, otherwise the result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018e670
     * @ghidraAddress PAL: 0x00195988
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};

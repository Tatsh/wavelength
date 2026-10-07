#pragma once

#include <vector>

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * The menu that chooses the memory card to manage the saves of.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003cc228`. The directional buttons step through the cards with saves, Cross opens the
 * choice between the remixes and the Freqs of the card, and Circle checks the connected cards
 * again.
 */
class ChooseMemCardScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no cards.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0016dbe0
     * @ghidraAddress PAL: 0x00170e08
     */
    explicit ChooseMemCardScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00356c58
     * @ghidraAddress PAL: 0x003c3eb8
     */
    static UIScreen *New(DataArray *pData) {
        return new ChooseMemCardScreen(pData);
    }

    /**
     * Route a controller button and the start of a choice, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0016ded8
     * @ghidraAddress PAL: 0x00171100
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter at the first card, and show it or show that no card has saves.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016dc28
     * @ghidraAddress PAL: 0x00170e50
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Step to the previous or the next card with the directional buttons.
     *
     * @param pMsg The message.
     * @return false.
     * @ghidraAddress NTSC-U/C: 0x0016df68
     * @ghidraAddress PAL: 0x00171190
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Check the cards again on Circle, or manage the chosen card on Cross, unless the screen is
     * between screens.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleJoypad(), or true between screens.
     * @ghidraAddress NTSC-U/C: 0x0016e0b8
     * @ghidraAddress PAL: 0x001712e0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    std::vector<int> mSlots; /*!< The memory card slots that have saves. */
    int mSlot;               /*!< The index into mSlots of the chosen card. */
};

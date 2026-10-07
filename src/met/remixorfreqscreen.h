#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The menu that chooses between the remixes and the Freqs of a memory card.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x80 bytes and its vtable
 * is at `0x003cc1c8`. ChooseMemCardScreen sets the card before it shows the menu.
 */
class RemixOrFreqScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0016e1e0
     * @ghidraAddress PAL: 0x00171408
     */
    explicit RemixOrFreqScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00356cf0
     * @ghidraAddress PAL: 0x003c3f50
     */
    static UIScreen *New(DataArray *pData) {
        return new RemixOrFreqScreen(pData);
    }

    /**
     * Route a chosen component, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0016e218
     * @ghidraAddress PAL: 0x00171440
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016e470
     * @ghidraAddress PAL: 0x001716a0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Open the copy and delete list of the remixes or of the Freqs of the card on Cross.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0016e280
     * @ghidraAddress PAL: 0x001714a8
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    int mSlot; /*!< The memory card slot. */
};

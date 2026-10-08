#pragma once

#include "met/selloadedfreqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The list of the Freqs of a memory card that copies or deletes one.
 *
 * The RTTI records the class as deriving from SelLoadedFreqScreen. The object is 0x190 bytes and
 * its vtable is at `0x003cc160`. Circle copies the chosen Freq to the other card and Square deletes
 * it. RemixOrFreqScreen sets the card before it shows the list.
 */
class FreqCopyDelScreen : public SelLoadedFreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0016e490
     * @ghidraAddress PAL: 0x001716c0
     */
    explicit FreqCopyDelScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00356ea8
     * @ghidraAddress PAL: 0x003c4108
     */
    static UIScreen *New(DataArray *pData) {
        return new FreqCopyDelScreen(pData);
    }

    /**
     * Route a controller button and a chosen component, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0016e4c8
     * @ghidraAddress PAL: 0x001716f8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, and list the Freqs.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016e5c8
     * @ghidraAddress PAL: 0x001717f8
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Keep the cursor of the list selected after a choice.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0016e558
     * @ghidraAddress PAL: 0x00171788
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Copy the chosen Freq on Circle or delete it on Square, unless the screen is between screens.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleJoypad(), or true between screens.
     * @ghidraAddress NTSC-U/C: 0x0016e5e8
     * @ghidraAddress PAL: 0x00171818
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mSlot; /*!< The memory card slot. */
};

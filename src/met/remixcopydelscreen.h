#pragma once

#include "met/remixselectscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The list of the remixes of a memory card that copies or deletes one.
 *
 * The RTTI records the class as deriving from RemixSelectScreen. The object is 0xa0 bytes and its
 * vtable is at `0x003cc0f8`. Circle copies the chosen remix to the other card and Square deletes
 * it. RemixOrFreqScreen sets the card before it shows the list.
 */
class RemixCopyDelScreen : public RemixSelectScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0016ea40
     * @ghidraAddress PAL: 0x00171c80
     */
    explicit RemixCopyDelScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00356fe8
     * @ghidraAddress PAL: 0x003c4248
     */
    static UIScreen *New(DataArray *pData) {
        return new RemixCopyDelScreen(pData);
    }

    /**
     * Route a controller button and a chosen component, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0016ea78
     * @ghidraAddress PAL: 0x00171cb8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Keep the cursor of the list selected after a choice.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0016eb08
     * @ghidraAddress PAL: 0x00171d48
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Copy the chosen remix on Circle or delete it on Square, unless the screen is between
     * screens.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleJoypad(), or true between screens.
     * @ghidraAddress NTSC-U/C: 0x0016eb78
     * @ghidraAddress PAL: 0x00171db8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mSlot; /*!< The memory card slot. */
};

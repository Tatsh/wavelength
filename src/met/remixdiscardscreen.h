#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The dialog that queries whether to discard the remix that was just made.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x74 bytes and its vtable
 * is at `0x003ce598`. The metagame registers the class for the screen type
 * `remix_discard_screen`.
 */
class RemixDiscardScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018b3f0
     * @ghidraAddress PAL: 0x00191ba8
     */
    explicit RemixDiscardScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `remix_discard_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035c890
     * @ghidraAddress PAL: 0x003ca968
     */
    static UIScreen *New(DataArray *pData) {
        return new RemixDiscardScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018b428
     * @ghidraAddress PAL: 0x00191be0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Go on from the answer chosen with the cross button.
     *
     * `yes` goes back to the remix mode menu, or in a local game to the save screen of the next
     * player. Every other answer goes back to the save screen. Online, `yes` goes back to the
     * launchpad.
     *
     * @param pMsg The message.
     * @return True for the cross button, otherwise the result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018b490
     * @ghidraAddress PAL: 0x00191c48
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    int mPlayer; /*!< The player whose save screen opened this one, from 1. */
};

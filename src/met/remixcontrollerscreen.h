#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"

/**
 * The screen that shows the controls of the remix editor before it starts.
 *
 * The RTTI records the class as deriving from FreqScreen, and its vtable is at `0x003ce4d8`. The
 * metagame registers the class for the screen type `remix_controller_screen`. The cross button
 * starts the sequence that loads the remix editor.
 */
class RemixControllerScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018b8c8
     * @ghidraAddress PAL: 0x00192080
     */
    explicit RemixControllerScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `remix_controller_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035ca40
     * @ghidraAddress PAL: 0x003cab18
     */
    static UIScreen *New(DataArray *pData) {
        return new RemixControllerScreen(pData);
    }

    /**
     * Route the controller buttons, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018b900
     * @ghidraAddress PAL: 0x001920b8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the solo or the multiplayer editor sequence when the cross button goes down.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise the result of
     * UIScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0018b968
     * @ghidraAddress PAL: 0x00192120
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};

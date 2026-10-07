#pragma once

#include "met/introscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"

/**
 * An intro screen the cross button cuts short.
 *
 * The RTTI records the class as deriving from IntroScreen. The object is 0x48 bytes and its
 * vtable is at `0x003cf018`. The metagame registers the class for the screen type `mktg_screen`.
 */
class MarketingScreen : public IntroScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018c388
     * @ghidraAddress PAL: 0x00192ec0
     */
    explicit MarketingScreen(DataArray *pData) : IntroScreen(pData) {
    }

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `mktg_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d0a8
     * @ghidraAddress PAL: 0x003cb180
     */
    static UIScreen *New(DataArray *pData) {
        return new MarketingScreen(pData);
    }

    /**
     * Route a controller button, and pass every other message to IntroScreen.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018c498
     * @ghidraAddress PAL: 0x00192fd0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * End a running hold at the next poll when the cross button goes down.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x0018c3c0
     * @ghidraAddress PAL: 0x00192ef8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};

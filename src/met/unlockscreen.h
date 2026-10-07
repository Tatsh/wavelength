#pragma once

#include "met/xproceedscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"

/**
 * Screen that reports an unlock and shows the next queued unlock when the cross button goes down.
 *
 * The RTTI records the class as deriving from XProceedScreen. The object is 0x70 bytes and its
 * vtable is at `0x003cf4b0`. The metagame registers the class for the screen type `unlock_screen`.
 * The destructor at `0x0035e808` is compiler-generated and has no declaration here.
 */
class UnlockScreen : public XProceedScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0035e8f0
     * @ghidraAddress PAL: 0x003cc9d0
     */
    explicit UnlockScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `unlock_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035e8b0
     * @ghidraAddress PAL: 0x003cc990
     */
    static UIScreen *New(DataArray *pData) {
        return new UnlockScreen(pData);
    }

    /**
     * Route a controller button to HandleJoypad(), and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00197590
     * @ghidraAddress PAL: 0x0019eab0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show the next queued unlock when the cross button goes down.
     *
     * A button that goes down while the screen enters or exits is swallowed.
     *
     * @param pMsg The message of the button.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00197508
     * @ghidraAddress PAL: 0x0019ea28
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};

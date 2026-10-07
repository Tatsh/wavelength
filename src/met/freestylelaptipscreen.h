#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"

/**
 * Screen that gives a tip about freestyle laps and shows the next queued unlock when the cross
 * button goes down.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003cf248`. The destructor at `0x0035ef68` is compiler-generated and has no declaration
 * here.
 */
class FreestyleLapTipScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0035f040
     * @ghidraAddress PAL: 0x003cd120
     */
    explicit FreestyleLapTipScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035f000
     * @ghidraAddress PAL: 0x003cd0e0
     */
    static UIScreen *New(DataArray *pData) {
        return new FreestyleLapTipScreen(pData);
    }

    /**
     * Route a controller button to HandleJoypad(), and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00198f50
     * @ghidraAddress PAL: 0x001a0470
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show the next queued unlock when the cross button goes down.
     *
     * A button that goes down while the screen enters or exits is swallowed.
     *
     * @param pMsg The message of the button.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00198ed0
     * @ghidraAddress PAL: 0x001a03f0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};

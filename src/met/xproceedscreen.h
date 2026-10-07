#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"

/**
 * Screen that the cross button leaves.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003cf0b8`.
 */
class XProceedScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0035cf00
     * @ghidraAddress PAL: 0x003cafd8
     */
    explicit XProceedScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Route a controller button to HandleJoypad(), and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018c090
     * @ghidraAddress PAL: 0x00192848
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Leave the screen when the cross button goes down.
     *
     * @param pMsg The message of the button.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018c0f8
     * @ghidraAddress PAL: 0x001928b0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};

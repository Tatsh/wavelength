#pragma once

#include "met/freqscreen.h"
#include "msg/inetdisconnectresultmsg.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that waits for the network connection to close and goes back to the configurations.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetInetDisconnect : public FreqScreen {
public:
    /**
     * Construct a screen.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x00177a80
     * @ghidraAddress PAL: 0x0017b268
     */
    explicit NetInetDisconnect(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00358e10
     * @ghidraAddress PAL: 0x003c6178
     */
    ~NetInetDisconnect() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetInetDisconnect(pData);
    }

    /**
     * Route the result of the disconnection and the end of the entry.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00177ab8
     * @ghidraAddress PAL: 0x0017b2a0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x00358ee8
     * @ghidraAddress PAL: 0x003c6250
     */
    const char *Title() override {
        return "";
    }

private:
    /**
     * Become the receiver of the network results once the screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00177b48
     * @ghidraAddress PAL: 0x0017b330
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Go back to the configurations.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00177ba8
     * @ghidraAddress PAL: 0x0017b390
     */
    bool HandleDisconnectResult(InetDisconnectResultMsg *pMsg);
};

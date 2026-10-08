#pragma once

#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "msg/joypadinputmsg.h"
#include "netflow/inetconnectresultmsg.h"
#include "netflow/inetconnectstatusmsg.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that connects to the network with a configuration of the memory card and shows the
 * progress in its dialog.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. The
 * object is 0xc0 bytes and its vtable is at `0x003cd710`. The screen first checks the memory card
 * that has the configuration.
 */
class NetInetConnect : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00177670
     * @ghidraAddress PAL: 0x0017ae48
     */
    explicit NetInetConnect(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00358ce0
     * @ghidraAddress PAL: 0x003c6048
     */
    ~NetInetConnect() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00358dc0
     */
    static UIScreen *New(DataArray *pData) {
        return new NetInetConnect(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x00358e00
     * @ghidraAddress PAL: 0x003c6168
     */
    const char *Title() override {
        return "";
    }

    /**
     * Connect once the memory card checks out, or show the memory card failure.
     *
     * @param nStatus The outcome of the check.
     * @ghidraAddress NTSC-U/C: 0x001776b8
     * @ghidraAddress PAL: 0x0017ae90
     */
    void OnCardStatus(int nStatus) override;

    /**
     * Route the connection messages, the end of a transition, and the controller to their
     * handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00177710
     * @ghidraAddress PAL: 0x0017aee8
     */
    bool DispatchPriv(Message *pMsg) override;

    int mReservedA4[3]; // +0xa4, not yet recovered.
    int mConfig;        /*!< The identifier of the configuration to connect with. +0xb0 */

private:
    /**
     * Check the memory card once the screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001777e0
     * @ghidraAddress PAL: 0x0017afb8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Show the step of the connection under way in the dialog.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00177830
     * @ghidraAddress PAL: 0x0017b008
     */
    bool HandleStatus(InetConnectStatusMsg *pMsg);

    /**
     * Go on to the lobby, or to the error screen with the message of the failure.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00177898
     * @ghidraAddress PAL: 0x0017b070
     */
    bool HandleResult(InetConnectResultMsg *pMsg);

    /**
     * Swallow a button during a transition, and cancel the connection on the triangle button.
     *
     * @param pMsg The message.
     * @return True during a transition, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00177a10
     * @ghidraAddress PAL: 0x0017b1f8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mConnecting;    // +0xb4, whether NetInet::Connect() is under way.
    int mReservedB8[2]; // +0xb8, not yet recovered.
};

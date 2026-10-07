#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "msg/lobbyconnectresultmsg.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that connects to the lobby server and goes on to the login, or shows the error.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetLobbyConnect : public FreqScreen {
public:
    /**
     * Construct a screen.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x00177bd8
     * @ghidraAddress PAL: 0x0017b3c0
     */
    explicit NetLobbyConnect(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00358ef8
     * @ghidraAddress PAL: 0x003c6260
     */
    ~NetLobbyConnect() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetLobbyConnect(pData);
    }

    /**
     * Route the result of the connection, the end of the entry, and the controller messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00177c10
     * @ghidraAddress PAL: 0x0017b450
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x00358fd0
     * @ghidraAddress PAL: 0x003c6338
     */
    const char *Title() override {
        return "";
    }

private:
    /**
     * Start the connection once the screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00177cc0
     * @ghidraAddress PAL: 0x0017b520
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Go on to the login, or show the error of the connection.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00177d20
     */
    bool HandleConnectResult(LobbyConnectResultMsg *pMsg);

    /**
     * Exit the lobby when the triangle button is pressed.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True while a transition runs, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00177e18
     * @ghidraAddress PAL: 0x0017b9f0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};

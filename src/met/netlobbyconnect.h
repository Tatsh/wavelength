#pragma once

#include "met/freqscreen.h"
#include "msg/dnasstatusmsg.h"
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
     * @ghidraAddress NTSC-U/C: 0x00358f90
     * @ghidraAddress PAL: 0x003c62f8
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

#ifdef VIDEO_STANDARD_PAL
    /**
     * Start the entry and hide the DNAS notice.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress PAL: 0x0017b3f8
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Start the exit.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress PAL: 0x0017b430
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;
#endif

private:
#ifdef VIDEO_STANDARD_PAL
    /**
     * Show the DNAS notice while the authentication runs.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress PAL: 0x0017b580
     */
    bool HandleDNASStatus(DNASStatusMsg *pMsg);

    /**
     * Show or hide the DNAS mesh and the text of the screen.
     *
     * The mesh shows only while the authentication runs and nShow is non-zero. The text shows
     * with nShow and reads the DNAS notice while the authentication runs.
     *
     * @param nRunning Non-zero while the authentication runs.
     * @param nShow Non-zero to show the text.
     * @ghidraAddress PAL: 0x0017b5a8
     */
    void ShowDNAS(int nRunning, int nShow);
#endif

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
     * @ghidraAddress PAL: 0x0017b750
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

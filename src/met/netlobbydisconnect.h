#pragma once

#include "met/freqscreen.h"
#include "msg/lobbydisconnectresultmsg.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that disconnects from the lobby server and goes back to the network portal.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetLobbyDisconnect : public FreqScreen {
public:
    /**
     * Construct a screen.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x00177e78
     * @ghidraAddress PAL: 0x0017ba50
     */
    explicit NetLobbyDisconnect(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00358fe0
     * @ghidraAddress PAL: 0x003c6348
     */
    ~NetLobbyDisconnect() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x00359078
     * @ghidraAddress PAL: 0x003c63e0
     */
    static UIScreen *New(DataArray *pData) {
        return new NetLobbyDisconnect(pData);
    }

    /**
     * Route the result of the disconnection and the end of the entry.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00177eb0
     * @ghidraAddress PAL: 0x0017ba88
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x003590b8
     * @ghidraAddress PAL: 0x003c6420
     */
    const char *Title() override {
        return "";
    }

private:
    /**
     * Start the disconnection once the screen has entered, noting whether the player asked for it.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00177f40
     * @ghidraAddress PAL: 0x0017bb18
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Go back to the network portal.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00177fd0
     */
    bool HandleDisconnectResult(LobbyDisconnectResultMsg *pMsg);

    int mAsked; /*!< Whether the player asked to disconnect, rather than an error. */
};

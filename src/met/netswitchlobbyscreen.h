#pragma once

#include "met/freqscreen.h"
#include "msg/joinchatroomresultmsg.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The dialog shown while this console moves to another chat room of the online lobby.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x74 bytes and its vtable
 * is at `0x003cdaa8`. The metagame registers the class for the screen type
 * `net_switch_lobby_screen`.
 */
class NetSwitchLobbyScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no room chosen.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001869f0
     * @ghidraAddress PAL: 0x0018ac30
     */
    explicit NetSwitchLobbyScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `net_switch_lobby_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035ba90
     * @ghidraAddress PAL: 0x003c95e8
     */
    static UIScreen *New(DataArray *pData) {
        return new NetSwitchLobbyScreen(pData);
    }

    /**
     * Route the result of the join and the end of the entry, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00186a28
     * @ghidraAddress PAL: 0x0018ac68
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035bad0
     * @ghidraAddress PAL: 0x003c9628
     */
    const char *Title() override {
        return "";
    }

    /**
     * Clear the focus of the lobby panel and join mChatroomId once the screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00186ab8
     * @ghidraAddress PAL: 0x0018acf8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Clear the lobby chat and record the room on success, then go to the success or the error
     * screen.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00186b50
     * @ghidraAddress PAL: 0x0018ad90
     */
    bool HandleJoinResult(JoinChatroomResultMsg *pMsg);

    int mChatroomId; /*!< The room to join, set by the screen that opens this one. */
};

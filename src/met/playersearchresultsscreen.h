#pragma once

#include "met/freqscreen.h"
#include "netflow/lobbyplayer.h"
#include "netflow/netchatroominfo.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen that shows the player a search found, and moves to the player's chatroom.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class PlayerSearchResultsScreen : public FreqScreen {
public:
    /**
     * Construct a screen with no player.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017de08
     * @ghidraAddress PAL: 0x00181ac8
     */
    explicit PlayerSearchResultsScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035ab80
     * @ghidraAddress PAL: 0x003c86d0
     */
    ~PlayerSearchResultsScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new PlayerSearchResultsScreen(pData);
    }

    /**
     * Route a choice to HandleSelect().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017e340
     * @ghidraAddress PAL: 0x00182000
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Pass the player to the `fn_find_p` panel, enter, and label the name, the chatroom, and the
     * buttons.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017de78
     * @ghidraAddress PAL: 0x00181b38
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Set the player to show and the chatroom the player is in.
     *
     * @param player The player.
     * @param chatroom The chatroom, with an identifier of -1 when the player is offline.
     */
    void SetPlayer(const LobbyPlayer &player, NetChatroomInfo chatroom) {
        mPlayer = player;
        mChatroomName = chatroom.mName.c_str();
        mChatroomId = chatroom.mId;
        mInChatroom = chatroom.mId != -1;
    }

private:
    /**
     * Return to the lobby with the `back` button, or move to the player's chatroom.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0017e3a8
     * @ghidraAddress PAL: 0x00182068
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    LobbyPlayer mPlayer;  /*!< The player. */
    String mChatroomName; /*!< The name of the chatroom the player is in. */
    int mChatroomId;      /*!< The chatroom the player is in, or -1. */
    int mInChatroom;      /*!< Non-zero when the player is in a chatroom. */
};

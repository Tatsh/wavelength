#pragma once

#include "met/avatarpanel.h"
#include "netflow/lobbyplayer.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel that shows the details and the Freq of a player a search found.
 *
 * The RTTI records the class as deriving from AvatarPanel.
 */
class NetFoundPlayerPanel : public AvatarPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00172b28
     * @ghidraAddress PAL: 0x00175f70
     */
    NetFoundPlayerPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00357f98
     * @ghidraAddress PAL: 0x003c5348
     */
    ~NetFoundPlayerPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00358058
     * @ghidraAddress PAL: 0x003c5408
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetFoundPlayerPanel(pData, pszDir);
    }

    /**
     * Start the entry and show the rank, the games, the connection, the presence, the chatroom,
     * the rank icon, and the Freq of the player.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00172b98
     * @ghidraAddress PAL: 0x00175fe0
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Set the player to show and the chatroom the player is in.
     *
     * @param player The player.
     * @param nChatroomId The chatroom, or -1 when the player is offline.
     * @param pszChatroom The name of the chatroom.
     */
    void SetPlayer(LobbyPlayer player, int nChatroomId, const char *pszChatroom) {
        mPlayer = player;
        mChatroomName = pszChatroom;
        mChatroomId = nChatroomId;
        mInChatroom = nChatroomId != -1;
    }

private:
    LobbyPlayer mPlayer;  /*!< The player. */
    int mReserved188;     // +0x188, not yet identified.
    int mInChatroom;      /*!< Non-zero when the player is in a chatroom. */
    int mChatroomId;      /*!< The chatroom the player is in, or -1. */
    String mChatroomName; /*!< The name of the chatroom the player is in. */
};

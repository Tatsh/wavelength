#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/lobbyplayer.h"
#include "netflow/netchatroominfo.h"

/**
 * Result of NetLobby::FindPlayer().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class FindPlayerMsg : public Message {
public:
    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     */
    Message *Clone() override;

    /**
     * Report this message's registered identity.
     *
     * @return g_nFindPlayerMsgType.
     */
    int Type() override {
        return g_nFindPlayerMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `FindPlayerMsg`.
     */
    const char *GetName() const override {
        return "FindPlayerMsg";
    }

    int mResult;               /*!< 0 when the player was found, otherwise a negative error. */
    LobbyPlayer mPlayer;       /*!< The player. */
    NetChatroomInfo mChatroom; /*!< The chatroom the player is in, with an identifier of -1 when the
                                  player is offline. */
};

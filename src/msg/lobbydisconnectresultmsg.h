#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Report that NetLobby::Disconnect() finished.
 *
 * The RTTI includes the class name and records Message as the base. The readers here use only its
 * identity.
 */
class LobbyDisconnectResultMsg : public Message {
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
     * @return g_nLobbyDisconnectResultMsgType.
     */
    int Type() override {
        return g_nLobbyDisconnectResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyDisconnectResultMsg`.
     */
    const char *GetName() const override {
        return "LobbyDisconnectResultMsg";
    }
};

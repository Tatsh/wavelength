#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Result of NetLobby::Connect().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class LobbyConnectResultMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultLostInternet = -91, /*!< The network connection was lost. */
        kResultSuccess = 0,        /*!< The lobby server is connected. */
    };

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     */
    Message *Clone() override;

    /**
     * Report this message's registered identity.
     *
     * @return g_nLobbyConnectResultMsgType.
     */
    int Type() override {
        return g_nLobbyConnectResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyConnectResultMsg`.
     */
    const char *GetName() const override {
        return "LobbyConnectResultMsg";
    }

    int mResult; /*!< One of Result, or another negative error. */
};

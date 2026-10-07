#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Report that the network connection was closed.
 *
 * The RTTI includes the class name and records Message as the base. The readers here use only its
 * identity.
 */
class InetDisconnectResultMsg : public Message {
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
     * @return g_nInetDisconnectResultMsgType.
     */
    int Type() override {
        return g_nInetDisconnectResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `InetDisconnectResultMsg`.
     */
    const char *GetName() const override {
        return "InetDisconnectResultMsg";
    }
};

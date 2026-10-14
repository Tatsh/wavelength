#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Message that reports the outcome of NetInet::CheckConnection().
 *
 * The RTTI records the class as deriving from Message. Only the members its readers here use are
 * declared.
 */
class InetCheckConnectionResultMsg : public Message {
public:
    int mResult; /*!< The outcome, zero while the connection works. */
};

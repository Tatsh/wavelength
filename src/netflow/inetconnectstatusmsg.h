#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/string.h"

/**
 * Message that reports the progress of NetInet::Connect().
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6480`. Only the
 * members its callers here use are declared.
 */
class InetConnectStatusMsg : public Message {
public:
    String mStatus; /*!< The localised text of the step under way. */
};

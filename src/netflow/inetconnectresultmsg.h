#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Message that reports the outcome of NetInet::Connect().
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6438`. Only the
 * members its callers here use are declared.
 */
class InetConnectResultMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultSuccess = 0,              /*!< The connection is up. */
        kResultConfigNotFound = -99,     /*!< The configuration is missing from the card. */
        kResultConfigWrongConsole = -98, /*!< The configuration belongs to another console. */
        kResultConfigError = -97,        /*!< The configuration cannot be read. */
        kResultHardwareError = -95,      /*!< The network adaptor failed. */
        kResultTimeout = -94,            /*!< The connection timed out. */
    };

    int mResult; /*!< The outcome, one of Result or another failure. */
};

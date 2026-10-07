#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Message that reports how much of the remix of an online game the consoles have exchanged.
 *
 * The RTTI records the class as deriving from Message. Only the members its receivers here read
 * are declared.
 */
class ShareRemixProgressMsg : public Message {
public:
    float mProgress; /*!< The part exchanged, from 0 to 1. */
    int mReceived;   /*!< Non-zero when this console received a remix. */
};

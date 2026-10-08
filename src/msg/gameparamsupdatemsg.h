#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netgameparams.h"

/**
 * Message that reports the settings of the online game the host published.
 *
 * The RTTI includes the class name and records Message as the base. Only the payload is declared.
 */
class GameParamsUpdateMsg : public Message {
public:
    NetGameParams mParams; /*!< The settings of the game. */
};

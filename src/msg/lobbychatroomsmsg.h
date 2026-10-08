#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netchatroominfo.h"

/**
 * Message that delivers the chat rooms of the lobby.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6168`. Only the
 * members its receivers here read are declared.
 */
class LobbyChatroomsMsg : public Message {
public:
    std::list<NetChatroomInfo> *mChatrooms; /*!< The chat rooms. */
};

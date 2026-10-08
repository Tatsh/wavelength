#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Message that reports the ladder position of a player whose position was requested.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d61f8`. Only the
 * members its receivers here read are declared.
 */
class PlayerRankMsg : public Message {
public:
    int mAccount; /*!< The account of the player. */
    int mRank;    /*!< The position of the player in the ranking. */
};

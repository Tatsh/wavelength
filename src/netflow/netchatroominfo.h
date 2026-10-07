#pragma once

#include "os/string.h"

/**
 * The description of one chat room of the online lobby.
 *
 * The record is 0x20 bytes and is copied member by member.
 */
struct NetChatroomInfo {
    int mId;            /*!< The identifier of the room, or -1 for none. */
    String mName;       /*!< The name of the room. */
    int mReserved18[2]; // +0x18, not yet recovered.
};

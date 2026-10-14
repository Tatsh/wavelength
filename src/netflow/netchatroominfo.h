#pragma once

#include "os/prnstream.h"
#include "os/string.h"

/**
 * The description of one chat room of the online lobby.
 *
 * The record is 0x20 bytes and is copied member by member.
 */
struct NetChatroomInfo {
    /**
     * Construct a record of no room.
     *
     * Inline. The counts are not set.
     */
    NetChatroomInfo() : mId(-1) {
    }

    int mId;          /*!< The identifier of the room, or -1 for none. */
    String mName;     /*!< The name of the room. */
    int mPlayerCount; /*!< The number of players in the room. */
    int mMaxPlayers;  /*!< The number of players the room takes. */
};

/**
 * Write a room as `id.name (players/max)`.
 *
 * @param stream The stream.
 * @param room The room.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00267de8
 * @ghidraAddress PAL: 0x00271ac0
 */
PrnStream &operator<<(PrnStream &stream, const NetChatroomInfo &room);

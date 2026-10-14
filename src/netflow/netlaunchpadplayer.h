#pragma once

#include "netflow/lobbyplayer.h"
#include "os/binstream.h"
#include "os/prnstream.h"

/**
 * The description of one player of the online session this console is in.
 *
 * The record is 0x94 bytes, the 0x9c-byte list nodes of NetLpadScreen::mPlayers less their links.
 * The network layer copies the records member by member.
 */
struct NetLaunchpadPlayer {
    LobbyPlayer mPlayer; /*!< The player. */
    int mId;             /*!< The player's identifier in the session, 0 for the host. */
    int mColor;          /*!< The colour of the player's slot, 0 to 3, or -1 for none. */
    int mReady;          /*!< The state of the player, 1 when the player is ready. */
};

/**
 * Write a player as its identifier, the LobbyPlayer, the colour, and the ready state.
 *
 * @param stream The stream.
 * @param player The player.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002682a0
 * @ghidraAddress PAL: 0x00271f78
 */
PrnStream &operator<<(PrnStream &stream, const NetLaunchpadPlayer &player);

/**
 * Write a player to the packets of a session.
 *
 * @param stream The stream.
 * @param player The player.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00268360
 * @ghidraAddress PAL: 0x00272038
 */
BinStream &operator<<(BinStream &stream, const NetLaunchpadPlayer &player);

/**
 * Read a player operator<<() wrote.
 *
 * @param stream The stream.
 * @param player Receives the player.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00268520
 * @ghidraAddress PAL: 0x002721b8
 */
BinStream &operator>>(BinStream &stream, NetLaunchpadPlayer &player);

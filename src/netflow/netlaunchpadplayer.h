#pragma once

#include "netflow/lobbyplayer.h"

/**
 * The description of one player of the online session this console is in.
 *
 * The record is 0x94 bytes, the 0x9c-byte list nodes of NetLpadScreen::mPlayers less their links.
 * The network layer copies the records member by member.
 */
struct NetLaunchpadPlayer {
    LobbyPlayer mPlayer; /*!< The player. */
    int mId;             /*!< The player's identifier in the session, 0 for the host. */
    int mDifficulty;     /*!< The difficulty the player chose, -1 for none. */
    int mReady;          /*!< 1 when the player is ready. */
};

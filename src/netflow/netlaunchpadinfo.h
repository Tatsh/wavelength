#pragma once

#include "netflow/netgameparams.h"

/**
 * The description of one online session that the lobby lists.
 *
 * The record is 0x64 bytes. The network layer builds one from each session the server reports and
 * copies them member by member.
 */
struct NetLaunchpadInfo {
    int mOpen;             /*!< Non-zero for a session, zero for an empty entry of the list. */
    int mLaunchpadId;      /*!< The first identifier NetJoinLPadScreen joins the session by. */
    int mLaunchpadWorld;   /*!< The second identifier NetJoinLPadScreen joins the session by. */
    NetGameParams mParams; /*!< The settings of the game. */
    int mConnectionType;   /*!< The kind of network connection of the host. */
    int mPlayerCount;      /*!< The number of players in the session. */
    int mStatus;           /*!< One of Status. */

    /** Values of mStatus. */
    enum Status {
        kStatusOpen = 0, /*!< The session takes players. */
        kStatusBusy = 1, /*!< The session is playing. */
        kStatusFull = 2, /*!< The session has no room. */
    };
};

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
    int mReserved58[3];    // +0x58, not yet recovered.
};

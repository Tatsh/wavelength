#pragma once

#include <list>

#include "netflow/netlaunchpadplayer.h"

/**
 * Mix-in for a panel of the launchpad screens that shows the players of the online session.
 *
 * The RTTI records the class with no base. NetLPadGamePanel, NetLPadPlayerPanel, and
 * NetHostLPadButtonPanel derive from it.
 */
class NetLPadPanel {
public:
    /**
     * Show the players of the session. Vtable slot 1.
     *
     * The name is inferred.
     *
     * @param pPlayers The players.
     */
    virtual void Update(std::list<NetLaunchpadPlayer> *pPlayers) = 0;
};

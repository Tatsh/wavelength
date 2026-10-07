#pragma once

#include <list>

#include "app/msgsink.h"
#include "netflow/netlaunchpadplayer.h"

/**
 * The online session a console hosts or joins, from its launchpad to the end of the song.
 *
 * The RTTI includes the class name. LaunchpadRT derives from it and from MsgSink, and its
 * constructor stores the one instance in TheNetLaunchpad. Only the table slots its callers here
 * use are declared.
 */
class NetLaunchpad {
public:
    /**
     * Vtable slot 1. Exit the session at the request of this console.
     *
     * LaunchpadRT ends the session with reason 0 and later sends pSink a LaunchpadAbortedMsg. The
     * name is inferred.
     *
     * @param pSink The sink that receives the end of the session.
     */
    virtual void Cancel(MsgSink *pSink) = 0;

    /**
     * Vtable slot 2. Report to the session that the launchpad screen shows.
     *
     * LaunchpadRT marks itself active. The name is inferred.
     */
    virtual void Activate() = 0;

    /**
     * Vtable slot 3. Report to the session that this console finished loading the song.
     *
     * LaunchpadRT moves its console's entry to state 3. The name is inferred.
     */
    virtual void NotifyLoaded() = 0;

    /**
     * Vtable slot 4. Report whether this console joined the session rather than hosting it.
     *
     * LaunchpadRT reports its word at `+0xac`. The name is inferred.
     *
     * @return Non-zero for a guest.
     */
    virtual int IsGuest() = 0;

    /**
     * Vtable slot 5. Report a count that changes whenever the players of the session change.
     *
     * The name is inferred.
     *
     * @return The count.
     */
    virtual int GetPlayersVersion() = 0;

    /**
     * Vtable slot 6. Report the players of the session.
     *
     * The name is inferred.
     *
     * @return The players.
     */
    virtual std::list<NetLaunchpadPlayer> *GetPlayers() = 0;

    /**
     * Vtable slot 7. Report that the host finished editing the remix of the game.
     *
     * LaunchpadRT does nothing. The name is inferred.
     */
    virtual void EndEdit() = 0;

    /**
     * Vtable slot 8. Offer the remix of the game to the guests.
     *
     * LaunchpadRT refuses. The name is inferred.
     *
     * @return Whether the offer started.
     */
    virtual bool ShareRemix() = 0;

    /**
     * Vtable slot 9. Start the song of the session on every console.
     *
     * LaunchpadRT does nothing and reports zero. The name is inferred.
     *
     * @return Zero in LaunchpadRT.
     */
    virtual int Launch() = 0;

    /**
     * Vtable slot 10. Remove a guest from the session.
     *
     * LaunchpadRT does nothing. The name is inferred.
     *
     * @param nPlayer The guest.
     */
    virtual void BootPlayer(int nPlayer) = 0;
};

/**
 * The online session, or null outside one.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ad0
 */
extern NetLaunchpad *TheNetLaunchpad;

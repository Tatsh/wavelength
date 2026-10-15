#pragma once

#include <list>

#include "app/msgsink.h"
#include "netflow/netlaunchpadplayer.h"

/**
 * The online session a console hosts or joins, from its launchpad to the end of the song.
 *
 * The RTTI includes the class name. LaunchpadRT derives from it and from MsgSink, and its
 * constructor stores the one instance in TheNetLaunchpad. The vtable is at `0x003d4850`. The
 * members of slots 7 to 11 have inline bodies, and the other members are pure.
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
     * Vtable slot 4. Report the identifier of this console in the session.
     *
     * The host is 0, and a guest is non-zero. The name is inferred.
     *
     * @return The identifier.
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
     * Vtable slot 7. Publish the settings of the game the host changed.
     *
     * The default does nothing. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00398be0
     * @ghidraAddress PAL: 0x00407468
     */
    virtual void EndEdit() {
    }

    /**
     * Vtable slot 8. Offer the remix of the game to the guests.
     *
     * The default refuses. The name is inferred.
     *
     * @return Whether the offer started.
     * @ghidraAddress NTSC-U/C: 0x00398be8
     * @ghidraAddress PAL: 0x00407470
     */
    virtual bool ShareRemix() {
        return false;
    }

    /**
     * Vtable slot 9. Start the song of the session on every console.
     *
     * The default does nothing and reports zero. The name is inferred.
     *
     * @return Non-zero when the song starts.
     * @ghidraAddress NTSC-U/C: 0x00398bf0
     * @ghidraAddress PAL: 0x00407478
     */
    virtual int Launch() {
        return 0;
    }

    /**
     * Vtable slot 10. Remove a guest from the session.
     *
     * The default does nothing. The name is inferred.
     *
     * @param nPlayer The guest.
     * @ghidraAddress NTSC-U/C: 0x00398bf8
     * @ghidraAddress PAL: 0x00407480
     */
    virtual void BootPlayer(int nPlayer) {
        (void)nPlayer;
    }

    /**
     * Vtable slot 11. Release the session.
     *
     * @ghidraAddress NTSC-U/C: 0x00398c00
     * @ghidraAddress PAL: 0x00407488
     */
    virtual ~NetLaunchpad() {
    }
};

/**
 * The online session, or null outside one.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ad0
 * @ghidraAddress PAL: 0x0042f99c
 */
extern NetLaunchpad *TheNetLaunchpad;

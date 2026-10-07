#pragma once

/**
 * The online session a console hosts or joins, from its launchpad to the end of the song.
 *
 * The RTTI includes the class name. LaunchpadRT derives from it and from MsgSink, and its
 * constructor stores the one instance in TheNetLaunchpad. Only the table slots its callers here
 * use are declared.
 */
class NetLaunchpad {
public:
    /** Vtable slot 1. The signature is not yet recovered. */
    virtual void VirtualSlot1() = 0;

    /** Vtable slot 2. The signature is not yet recovered. */
    virtual void VirtualSlot2() = 0;

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
};

/**
 * The online session, or null outside one.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ad0
 */
extern NetLaunchpad *TheNetLaunchpad;

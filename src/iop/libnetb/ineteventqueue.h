#pragma once

#include <inetctl.h>

/**
 * Queue of the inetctl events. An event handler fills it and the EE drains it through RPC
 * functions. It also records the interfaces that started.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class InetEventQueue {
public:
    /** Interface states MapEvent() reports. */
    enum InterfaceState {
        kInterfaceNone = -1,    /*!< The event changes nothing the EE waits for. */
        kInterfaceUp = 1,       /*!< The primary interface came up. */
        kInterfaceDown = 4,     /*!< An interface went down. */
        kInterfaceLost = 0x100, /*!< The primary interface went away. */
    };

    /** Number of recorded interfaces. */
    static constexpr int kInterfaceCount = 2;

    /** Capacity of the queue. */
    static constexpr int kQueueSize = 16;

    /**
     * Take the oldest event, waiting up to five times 0.3 seconds for the first one.
     *
     * @param interfaceId Receives the interface identifier, -1 after a queue overflow.
     * @param event Receives the event.
     * @return Zero, #kLibnetErrorNoEvent, or #kLibnetErrorSemaphore.
     * @ghidraAddress NTSC-U/C: 0x000001ec
     * @ghidraAddress PAL: 0x000001ec
     */
    static int GetEvent(int *interfaceId, int *event);

    /**
     * Turn an event into an InterfaceState, recording a started interface.
     *
     * @param interfaceId Interface identifier.
     * @param event Event.
     * @return An InterfaceState.
     * @ghidraAddress NTSC-U/C: 0x00000358
     * @ghidraAddress PAL: 0x00000358
     */
    static int MapEvent(int interfaceId, int event);

    /**
     * Event handler. Append an event to the queue.
     *
     * @param interfaceId Interface identifier.
     * @param event Event.
     * @ghidraAddress NTSC-U/C: 0x00000000
     * @ghidraAddress PAL: 0x00000000
     */
    static void EventHandler(int interfaceId, int event);

    /** Recorded interfaces. The first is the one the module waits for. */
    static int sInterfaceIds[kInterfaceCount];

    /** Semaphore that guards the queue. */
    static int sLockSema;

    /** Semaphore that counts the queued events. */
    static int sCountSema;

    /** Index of the oldest event. */
    static int sReadIndex;

    /** Index the next event goes to. */
    static int sWriteIndex;

    /** Flag bits. */
    static unsigned int sFlags;

    /** Registration of EventHandler(). */
    static sceInetCtlEventHandler sHandler;

private:
    /** Bits of sFlags. */
    enum Flag {
        kFlagEventReceived = 0x01, /*!< At least one event arrived. */
        kFlagOverflow = 0x02,      /*!< The queue overflowed. */
    };

    /**
     * Record a started interface. An interface with #INET_INTERFACE_PASSIVE takes the first free
     * secondary slot, and any other one the primary slot.
     *
     * @param interfaceId Interface identifier.
     * @ghidraAddress NTSC-U/C: 0x00000110
     * @ghidraAddress PAL: 0x00000110
     */
    static void RecordInterface(int interfaceId);

    /**
     * Report whether an interface is the recorded primary one.
     *
     * @param interfaceId Interface identifier.
     * @return #kInterfaceUp when it is, otherwise #kInterfaceNone.
     * @ghidraAddress NTSC-U/C: 0x00000194
     * @ghidraAddress PAL: 0x00000194
     */
    static int IsPrimaryInterface(int interfaceId);

    /** Interface identifiers of the queued events. */
    static int sInterfaceIdQueue[kQueueSize];

    /** The queued events. */
    static int sEventQueue[kQueueSize];
};

#pragma once

#include "app/msgsink.h"

/**
 * Network connection of the console.
 *
 * The RTTI includes the class name, and InetImpl derives from it. Only the member its callers
 * here use is declared.
 */
class NetInet {
public:
    /** Values GetConnectionType() reports. */
    enum ConnectionType {
        kConnectionSlow = 1, /*!< A connection the game gives more time per packet. */
    };

    /** Release the connection. */
    virtual ~NetInet();

    /**
     * Load the network modules. Vtable slot 2. The name is inferred.
     */
    virtual void LoadModules() = 0;

    /**
     * Start searching the memory cards for the network configurations. Vtable slot 3.
     *
     * The sink later receives an InetConfigsResultMsg. A search already running is not restarted.
     * The name is inferred.
     *
     * @param pSink The receiver of the result.
     */
    virtual void RequestConfigs(MsgSink *pSink) = 0;

    /**
     * Report the kind of connection.
     *
     * @return The kind.
     */
    virtual int GetConnectionType() = 0;
};

/**
 * The network connection.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ab4
 */
extern NetInet *TheNetInet;

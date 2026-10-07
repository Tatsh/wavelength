#pragma once

#include "app/msgsink.h"

/**
 * Network connection of the console.
 *
 * The RTTI includes the class name, and InetImpl derives from it. The members report their results
 * to the receiver SetSink() sets, as messages such as InetConfigsResultMsg and
 * InetConnectResultMsg. The member names other than GetConnectionType() are inferred.
 */
class NetInet {
public:
    /** Values GetConnectionType() reports. */
    enum ConnectionType {
        kConnectionSlow = 1, /*!< A connection the game gives more time per packet. */
    };

    /** Release the connection. */
    virtual ~NetInet();

    /** Start the program that creates a network configuration. */
    virtual void LaunchConfigTool() = 0;

    /** Start listing the network configurations of the memory card. */
    virtual void RequestConfigs() = 0;

    /**
     * Start connecting with a network configuration.
     *
     * @param pSink The receiver of the InetConnectStatusMsg and InetConnectResultMsg.
     * @param nConfig The identifier of the configuration.
     */
    virtual void Connect(MsgSink *pSink, int nConfig) = 0;

    /**
     * Set the receiver of the result messages.
     *
     * @param pSink The receiver, or null.
     */
    virtual void SetSink(MsgSink *pSink) = 0;

    /**
     * Start looking up the address of a host.
     *
     * @param pszHost The host name.
     */
    virtual void LookupHost(const char *pszHost) = 0;

    /** Clear the pending request. */
    virtual void ClearPending() = 0;

    /** Start checking whether the connection still works. */
    virtual void CheckConnection() = 0;

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

    /** Mark the connection ready. */
    virtual void SetReady() = 0;
};

/**
 * The network connection.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ab4
 */
extern NetInet *TheNetInet;

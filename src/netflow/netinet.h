#pragma once

#include "app/msgsink.h"

/**
 * Network connection of the console.
 *
 * The RTTI includes the class name, and InetImpl derives from it. The members report their results
 * to a receiver as messages such as InetConfigsResultMsg and InetConnectResultMsg. The vtable of
 * InetImpl at `0x003d43f0` gives the order of the members. The member names other than
 * GetConnectionType() are inferred.
 */
class NetInet {
public:
    /** Values GetConnectionType() reports. */
    enum ConnectionType {
        kConnectionSlow = 1, /*!< A connection the game gives more time per packet. */
    };

    /** Vtable slot 1. Release the connection. */
    virtual ~NetInet();

    /** Vtable slot 2. Start the program that creates a network configuration. */
    virtual void LaunchConfigTool() = 0;

    /**
     * Vtable slot 3. Start searching the memory cards for the network configurations.
     *
     * The sink later receives an InetConfigsResultMsg. A search already running is not restarted.
     *
     * @param pSink The receiver of the result.
     */
    virtual void RequestConfigs(MsgSink *pSink) = 0;

    /**
     * Vtable slot 4. Start connecting with a network configuration.
     *
     * @param pSink The receiver of the InetConnectStatusMsg and InetConnectResultMsg.
     * @param nConfig The identifier of the configuration.
     */
    virtual void Connect(MsgSink *pSink, int nConfig) = 0;

    /**
     * Vtable slot 5. Set the receiver of the result messages.
     *
     * @param pSink The receiver, or null.
     */
    virtual void SetSink(MsgSink *pSink) = 0;

    /**
     * Vtable slot 6. Start looking up the address of a host.
     *
     * @param pSink The receiver of the InetDNSLookupResultMsg.
     * @param pszHost The host name.
     */
    virtual void LookupHost(MsgSink *pSink, const char *pszHost) = 0;

    /** Vtable slot 7. Clear the pending lookup. */
    virtual void ClearPending() = 0;

    /**
     * Vtable slot 8. Check whether the connection still works.
     *
     * @param pSink The receiver of the InetCheckConnectionResultMsg.
     */
    virtual void CheckConnection(MsgSink *pSink) = 0;

    /**
     * Vtable slot 9. Report the kind of connection.
     *
     * @return The kind.
     */
    virtual int GetConnectionType() = 0;

    /** Vtable slot 10. Mark the connection ready. */
    virtual void SetReady() = 0;
};

/**
 * The network connection.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ab4
 */
extern NetInet *TheNetInet;

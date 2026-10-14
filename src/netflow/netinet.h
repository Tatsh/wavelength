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
        kConnectionSlow = 1,    /*!< A modem, which the game gives more time per packet. */
        kConnectionFast = 2,    /*!< An Ethernet adaptor. */
        kConnectionOffline = 3, /*!< No interface is up. */
    };

    /**
     * Vtable slot 1. Release the connection.
     *
     * Inline.
     *
     * @ghidraAddress NTSC-U/C: 0x003974d0
     */
    virtual ~NetInet() {
    }

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
     * Vtable slot 5. Take the interface down once no connection is starting.
     *
     * @param pSink The receiver of the InetDisconnectResultMsg.
     */
    virtual void Disconnect(MsgSink *pSink) = 0;

    /**
     * Vtable slot 6. Start looking up the address of a host.
     *
     * @param pSink The receiver of the InetDNSLookupResultMsg.
     * @param pszHost The host name.
     */
    virtual void LookupHost(MsgSink *pSink, const char *pszHost) = 0;

    /** Vtable slot 7. Forget the receiver of the pending lookup. */
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
     * @return One of ConnectionType.
     */
    virtual int GetConnectionType() = 0;

    /**
     * Vtable slot 10. Turn the console off at the next poll.
     *
     * The default does nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x00397500
     */
    virtual void RequestPowerOff() {
    }
};

/**
 * The network connection.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ab4
 */
extern NetInet *TheNetInet;

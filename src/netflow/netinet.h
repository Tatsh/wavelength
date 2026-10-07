#pragma once

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

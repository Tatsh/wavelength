#pragma once

#include <vector>

#include "app/msgsink.h"
#include "game/netgamescore.h"
#include "msg/externalpacket.h"

/**
 * Connection that carries packets between consoles.
 *
 * The RTTI includes the class name. TransportRT derives from it, and its constructor stores the
 * one instance in TheNetTransport. Only the members its callers here use are declared.
 */
class NetTransport {
public:
    /**
     * Set the sink the received messages go to.
     *
     * TransportRT's table places the member first. The name is inferred.
     *
     * @param pSink The sink, or null.
     */
    virtual void SetSink(MsgSink *pSink) = 0;

    /** Release the transport. */
    virtual ~NetTransport();

    /**
     * Encode a packet and send it to the peers its receiver selects.
     *
     * TransportRT's implementation sends nothing while no session is open.
     *
     * @param packet The packet.
     */
    virtual void Send(ExternalPacket &packet) = 0;

    /**
     * Report whether this console hosts the session.
     *
     * @return Whether this console is the host.
     */
    virtual bool IsHost() = 0;

    /** Leave the session. */
    virtual void Leave() = 0;

    /**
     * Report the final scores of the session to the session server.
     *
     * @param scores The score of each player.
     */
    virtual void ReportScores(const std::vector<NetGameScore> &scores) = 0;
};

/**
 * The network transport.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0cac
 */
extern NetTransport *TheNetTransport;

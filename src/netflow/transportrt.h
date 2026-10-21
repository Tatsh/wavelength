#pragma once

#include <memory>
#include <vector>

#include "app/msgsink.h"
#include "game/netgamescore.h"
#include "msg/externalpacket.h"
#include "netflow/nettransport.h"

class Message;
class Packet;

/**
 * The transport of the packets of the game over the online session of LaunchpadRT.
 *
 * The RTTI includes the class name and records NetTransport as the base. The vtable is at
 * `0x003d6510`, and the object is 0x1c bytes. The constructor stores the instance in
 * TheTransportRT and TheNetTransport. The transport passes packets on only while it is open.
 */
class TransportRT : public NetTransport {
public:
    /**
     * Construct a closed transport with no sink.
     *
     * @ghidraAddress NTSC-U/C: 0x0026b088
     * @ghidraAddress PAL: 0x00274c18
     */
    TransportRT();

    /**
     * Clear TheTransportRT and TheNetTransport.
     *
     * @ghidraAddress NTSC-U/C: 0x0026b0f0
     * @ghidraAddress PAL: 0x00274c80
     */
    ~TransportRT() override;

    /**
     * Set mSink.
     *
     * @param pSink The sink, or null.
     * @ghidraAddress NTSC-U/C: 0x0026b3f8
     * @ghidraAddress PAL: 0x00274f88
     */
    void SetSink(MsgSink *pSink) override;

    /**
     * Send a packet through the session while the transport is open.
     *
     * @param packet The packet.
     * @ghidraAddress NTSC-U/C: 0x0026b400
     * @ghidraAddress PAL: 0x00274f90
     */
    void Send(ExternalPacket &packet) override;

    /**
     * Report whether this console hosts the session.
     *
     * @return Whether a session exists and this console has the identifier of the host.
     * @ghidraAddress NTSC-U/C: 0x0026b438
     * @ghidraAddress PAL: 0x00274fc8
     */
    bool IsHost() override;

    /**
     * Leave the game of the session.
     *
     * @ghidraAddress NTSC-U/C: 0x0026b458
     * @ghidraAddress PAL: 0x00274fe8
     */
    void Leave() override;

    /**
     * Send the final scores of the game from the host.
     *
     * @param scores The score of each player.
     * @ghidraAddress NTSC-U/C: 0x0026b480
     * @ghidraAddress PAL: 0x00275010
     */
    void ReportScores(const std::vector<NetGameScore> &scores) override;

    /**
     * Pass a message to mSink while the transport is open.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x0026b178
     * @ghidraAddress PAL: 0x00274d08
     */
    void Notify(Message *pMsg);

    /**
     * Pass a received packet to mSink while the transport is open, unless its sender left.
     *
     * The name is inferred.
     *
     * @param pPacket The packet.
     * @ghidraAddress NTSC-U/C: 0x0026b1b8
     * @ghidraAddress PAL: 0x00274d48
     */
    void Receive(Packet *pPacket);

    /**
     * Note a player who left, and report the player to mSink while the transport is open.
     *
     * The name is inferred.
     *
     * @param nId The player's identifier in the session.
     * @ghidraAddress NTSC-U/C: 0x0026b240
     * @ghidraAddress PAL: 0x00274dd0
     */
    void PlayerAborted(int nId);

    /**
     * Open or close the transport. Opening forgets the players who left.
     *
     * The name is inferred.
     *
     * @param nOpen Non-zero to open the transport.
     * @ghidraAddress NTSC-U/C: 0x0026b3e0
     * @ghidraAddress PAL: 0x00274f70
     */
    void SetOpen(int nOpen);

    /** @return The sink the messages of the game go to, or null. Borrowed. */
    MsgSink *GetSink() const {
        return mSink;
    }

    /** @return Non-zero while the game runs. */
    int GetOpen() const {
        return mOpen;
    }

    /** @return The players who exited the game. */
    const std::vector<int> &GetAborted() const {
        return mAborted;
    }

private:
    MsgSink *mSink;            /*!< The sink the messages of the game go to, or null. Borrowed. */
    int mOpen;                 /*!< Non-zero while the game runs. */
    std::vector<int> mAborted; /*!< The players who exited the game. */
};

/**
 * The transport.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ca8
 * @ghidraAddress PAL: 0x0042fb78
 */
extern std::unique_ptr<TransportRT> TheTransportRT;

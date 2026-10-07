#pragma once

#include "msg/packet.h"

/**
 * Base of a packet a peer sends to another console.
 *
 * The RTTI records Packet as the one base. The class adds no member. Every packet constructor
 * marks the sender unset, and NetTransport::Send() reads the receiver and the delivery flags.
 */
class ExternalPacket : public Packet {
public:
    /**
     * Construct a packet from an unset sender.
     *
     * Inline. Every packet constructor expands it.
     *
     * @param nFlags The delivery flags.
     * @param nReceiver The peer the packet is for, or kAllPeers.
     */
    explicit ExternalPacket(unsigned char nFlags = 0, int nReceiver = kAllPeers)
        : Packet(nFlags, nReceiver) {
    }
};

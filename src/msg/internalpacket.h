#pragma once

#include "msg/packet.h"

/**
 * Base of a packet the consoles of an online session exchange to run the session itself.
 *
 * The RTTI records Packet as the one base. The class adds no member. A received packet of the
 * class goes to the online session rather than to the game.
 */
class InternalPacket : public Packet {
public:
    /**
     * Report that a received packet goes to the online session.
     *
     * Inline. Every derived vtable addresses a shared copy at `0x00398e48` that returns 2.
     *
     * @return kRouteLaunchpad.
     */
    int Route() override {
        return kRouteLaunchpad;
    }

protected:
    /**
     * Construct a packet from an unset sender.
     *
     * Inline. Every packet constructor expands it.
     *
     * @param nFlags The delivery flags.
     * @param nReceiver The peer the packet is for, kAllPeers, or kUnset.
     */
    explicit InternalPacket(unsigned char nFlags = 0, int nReceiver = kUnset)
        : Packet(nFlags, nReceiver) {
    }
};

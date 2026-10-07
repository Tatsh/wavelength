#pragma once

#include "msg/message.h"

/**
 * Abstract base of a message that travels between consoles.
 *
 * The RTTI records Message as the one base. ExternalPacket and InternalPacket derive from it, and
 * the image has no vtable of this class. The class has no out-of-line routine. Every concrete
 * packet's allocation stores the three members below, the network receive routine at
 * `0x00256440` writes the sender after its cast to this class, and the receive routine then calls
 * a virtual this class introduces whose purpose is not yet identified.
 */
class Packet : public Message {
public:
    /** Value of mSender and mReceiver that marks the member as unset. */
    static constexpr int kUnset = -2;

    /** Value of mReceiver that addresses every peer. */
    static constexpr int kAllPeers = -1;

    /** Bit of mFlags that NetTransport::Send() turns into its guaranteed delivery bit. */
    static constexpr unsigned char kFlagGuaranteed = 2;

    int mSender;          /*!< The peer that sent the packet, or kUnset. */
    int mReceiver;        /*!< The peer the packet is for, kAllPeers, or kUnset. */
    unsigned char mFlags; /*!< Delivery flags NetTransport::Send() translates. */

protected:
    /**
     * Construct a packet from an unset sender.
     *
     * Inline. Every packet allocation expands it.
     *
     * @param nFlags The delivery flags.
     * @param nReceiver The peer the packet is for, kAllPeers, or kUnset.
     */
    Packet(unsigned char nFlags, int nReceiver)
        : mSender(kUnset), mReceiver(nReceiver), mFlags(nFlags) {
    }
};

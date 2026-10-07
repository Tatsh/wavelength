#pragma once

#include "msg/message.h"

/**
 * Base of a message that travels between consoles.
 *
 * The RTTI records Packet as the base and Message as the base of Packet. Packet is not yet ported
 * from its earlier layout, so the class derives from Message directly, and the sender and receiver
 * words Packet provides are declared here. The object is the vptr and the three members below.
 * Every packet constructor marks the sender unset, and NetTransport::Send() reads the receiver and
 * the delivery flags.
 */
class ExternalPacket : public Message {
public:
    /** Value of mSender and mReceiver that marks the member as unset. */
    static constexpr int kUnset = -2;

    /** Value of mReceiver that addresses every peer. */
    static constexpr int kAllPeers = -1;

    /** Bit of mFlags that NetTransport::Send() turns into its guaranteed delivery bit. */
    static constexpr unsigned char kFlagGuaranteed = 2;

    /**
     * Construct a packet from an unset sender.
     *
     * Inline. Every packet constructor expands it.
     *
     * @param nFlags The delivery flags.
     * @param nReceiver The peer the packet is for, or kAllPeers.
     */
    explicit ExternalPacket(unsigned char nFlags = 0, int nReceiver = kAllPeers)
        : mSender(kUnset), mReceiver(nReceiver), mFlags(nFlags) {
    }

    int mSender;          /*!< The peer that sent the packet, or kUnset. */
    int mReceiver;        /*!< The peer the packet is for, kAllPeers, or kUnset. */
    unsigned char mFlags; /*!< Delivery flags NetTransport::Send() translates. */
};

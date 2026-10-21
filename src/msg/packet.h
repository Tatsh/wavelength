#pragma once

#include "msg/message.h"

/**
 * Abstract base of a message that travels between consoles.
 *
 * The RTTI records Message as the one base. ExternalPacket and InternalPacket derive from it. Its
 * vtable at `0x003d5d70` leaves Clone(), Type(), GetName(), and Route() pure. Every concrete
 * packet's allocation stores the three members below, and the network receive routine at
 * `0x00256440` writes the sender after its cast to this class.
 */
class Packet : public Message {
public:
    /** Value of mSender and mReceiver that marks the member as unset. */
    static constexpr int kUnset = -2;

    /** Value of mReceiver that addresses every peer. */
    static constexpr int kAllPeers = -1;

    /**
     * Bit of mFlags that LaunchpadRT::Send() turns into the second delivery bit of the session
     * library. The name is inferred.
     */
    static constexpr unsigned char kFlagOrdered = 1;

    /** Bit of mFlags that NetTransport::Send() turns into its guaranteed delivery bit. */
    static constexpr unsigned char kFlagGuaranteed = 2;

    /** Bits of Route(). */
    enum RouteFlag {
        kRouteTransport = 1, /*!< The packet goes to the sink of TheNetTransport. */
        kRouteLaunchpad = 2, /*!< The packet goes to the online session itself. */
    };

    /**
     * Write the sender and the receiver that are set, as `from:` and `to:`.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269200
     * @ghidraAddress PAL: 0x00272e08
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Report the receivers a received packet goes to.
     *
     * The receive routine passes the packet to every receiver whose RouteFlag the result sets.
     * The name is inferred.
     *
     * @return A combination of RouteFlag.
     */
    virtual int Route() = 0;

    /** @return Delivery flags NetTransport::Send() translates. */
    unsigned char GetFlags() const {
        return mFlags;
    }

    /** @return The peer the packet is for, kAllPeers, or kUnset. */
    int GetReceiver() const {
        return mReceiver;
    }

    /** @return The peer that sent the packet, or kUnset. */
    int GetSender() const {
        return mSender;
    }

    /** @param nSender The peer that sent the packet, or kUnset. */
    void SetSender(int nSender) {
        mSender = nSender;
    }

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

private:
    int mSender;          /*!< The peer that sent the packet, or kUnset. */
    int mReceiver;        /*!< The peer the packet is for, kAllPeers, or kUnset. */
    unsigned char mFlags; /*!< Delivery flags NetTransport::Send() translates. */
};

#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that FreestyleBumpPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af73c
 */
extern int g_nFreestyleBumpPacketType;

/**
 * Packet that reports a player bumped onto a track by a freestyle power-up.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline.
 */
class FreestyleBumpPacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no player and no track.
     *
     * Inline. New() expands it.
     */
    FreestyleBumpPacket() : ExternalPacket(kFlagGuaranteed), mNetOrder(-1), mTrack(-1) {
    }

    /**
     * Construct a packet for a player and a track.
     *
     * Inline. NetArbiter::UpdatePlayer() expands it on its stack.
     *
     * @param nNetOrder The session order of the player bumped.
     * @param nTrack The track the player moves to.
     */
    FreestyleBumpPacket(int nNetOrder, int nTrack)
        : ExternalPacket(kFlagGuaranteed), mNetOrder(nNetOrder), mTrack(nTrack) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x001231a0
     * @ghidraAddress PAL: 0x00124920
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "FreestyleBumpPacket", 0);
    }

    /**
     * Release a packet.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Produce a heap copy of this packet.
     *
     * @return The copy.
     * @ghidraAddress NTSC-U/C: 0x0034b500
     * @ghidraAddress PAL: 0x003b8930
     */
    Message *Clone() override {
        return new FreestyleBumpPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nFreestyleBumpPacketType.
     * @ghidraAddress NTSC-U/C: 0x0034b578
     * @ghidraAddress PAL: 0x003b89a8
     */
    int Type() override {
        return g_nFreestyleBumpPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `FreestyleBumpPacket`.
     * @ghidraAddress NTSC-U/C: 0x0034b588
     * @ghidraAddress PAL: 0x003b89b8
     */
    const char *GetName() const override {
        return "FreestyleBumpPacket";
    }

    int mNetOrder; /*!< The session order of the player bumped. */
    int mTrack;    /*!< The track the player was moved to. */
};

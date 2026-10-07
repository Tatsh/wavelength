#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that CripplerPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af72c
 */
extern int g_nCripplerPacketType;

/**
 * Packet that reports the player a crippler power-up struck.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline.
 */
class CripplerPacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no victim.
     *
     * Inline. New() expands it.
     */
    CripplerPacket() : ExternalPacket(kFlagGuaranteed), mVictim(-1) {
    }

    /**
     * Construct a packet for a victim.
     *
     * Inline. MultiGameLogic::DeployCrippler() expands it on its stack.
     *
     * @param nVictim The session order of the player struck.
     */
    explicit CripplerPacket(int nVictim) : ExternalPacket(kFlagGuaranteed), mVictim(nVictim) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00123040
     * @ghidraAddress PAL: 0x001247c0
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "CripplerPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0034b0c8
     * @ghidraAddress PAL: 0x003b84f8
     */
    Message *Clone() override {
        return new CripplerPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nCripplerPacketType.
     * @ghidraAddress NTSC-U/C: 0x0034b138
     * @ghidraAddress PAL: 0x003b8568
     */
    int Type() override {
        return g_nCripplerPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `CripplerPacket`.
     * @ghidraAddress NTSC-U/C: 0x0034b148
     * @ghidraAddress PAL: 0x003b8578
     */
    const char *GetName() const override {
        return "CripplerPacket";
    }

    /**
     * Write the victim as one byte.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0014e790
     * @ghidraAddress PAL: 0x00150130
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the byte saveGuts() writes back into the victim.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0014e7d0
     */
    void restoreGuts(BinStream &stream) override;

    int mVictim; /*!< The session order of the player struck. */
};

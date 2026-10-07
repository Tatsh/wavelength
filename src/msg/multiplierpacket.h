#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that MultiplierPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af734
 */
extern int g_nMultiplierPacketType;

/**
 * Packet that announces a player's multiplier power-up to the other consoles.
 *
 * The RTTI records the class as deriving from ExternalPacket. The class adds no payload. The
 * members Clone(), Type(), and GetName() are inline.
 */
class MultiplierPacket : public ExternalPacket {
public:
    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x001230f0
     * @ghidraAddress PAL: 0x00124870
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "MultiplierPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0033acc8
     * @ghidraAddress PAL: 0x003a8200
     */
    Message *Clone() override {
        return new MultiplierPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nMultiplierPacketType.
     * @ghidraAddress NTSC-U/C: 0x0033ad30
     * @ghidraAddress PAL: 0x003a8268
     */
    int Type() override {
        return g_nMultiplierPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `MultiplierPacket`.
     * @ghidraAddress NTSC-U/C: 0x0033ad40
     * @ghidraAddress PAL: 0x003a8278
     */
    const char *GetName() const override {
        return "MultiplierPacket";
    }
};

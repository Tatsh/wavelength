#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that SlowdownPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af730
 */
extern int g_nSlowdownPacketType;

/**
 * Packet that announces a slowdown power-up to the other consoles.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed, and the
 * class adds no payload. The members Clone(), Type(), and GetName() are inline.
 */
class SlowdownPacket : public ExternalPacket {
public:
    /**
     * Construct a packet.
     *
     * Inline. New() expands it.
     */
    SlowdownPacket() : ExternalPacket(kFlagGuaranteed) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00123098
     * @ghidraAddress PAL: 0x00124818
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "SlowdownPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0033b378
     * @ghidraAddress PAL: 0x003a88b0
     */
    Message *Clone() override {
        return new SlowdownPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nSlowdownPacketType.
     * @ghidraAddress NTSC-U/C: 0x0033b3e0
     * @ghidraAddress PAL: 0x003a8918
     */
    int Type() override {
        return g_nSlowdownPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `SlowdownPacket`.
     * @ghidraAddress NTSC-U/C: 0x0033b3f0
     * @ghidraAddress PAL: 0x003a8928
     */
    const char *GetName() const override {
        return "SlowdownPacket";
    }
};

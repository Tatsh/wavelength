#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that DataAckPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0bb4
 */
extern int g_nDataAckPktType;

/**
 * Packet a guest sends the host to acknowledge a DataBlockPkt.
 *
 * The RTTI records the class as deriving from InternalPacket, and its vtable is at `0x003d4500`.
 * Delivery is guaranteed, to the host. The class adds no member, and its payload routines are
 * those of Packet. The members Clone(), Type(), and GetName() are inline.
 */
class DataAckPkt : public InternalPacket {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00398e50
     * @ghidraAddress PAL: 0x004076d8
     */
    ~DataAckPkt() override = default;

    /** The identifier in the session of the host, the receiver of every packet of the class. */
    static constexpr int kHost = 0;

    /**
     * Construct a packet for the host.
     *
     * Inline. New() expands it.
     */
    DataAckPkt() : InternalPacket(kFlagGuaranteed, kHost) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267860
     * @ghidraAddress PAL: 0x00271538
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "DataAckPkt", 0);
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
     * @ghidraAddress NTSC-U/C: 0x00398ed0
     * @ghidraAddress PAL: 0x00407758
     */
    Message *Clone() override {
        return new DataAckPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nDataAckPktType.
     * @ghidraAddress NTSC-U/C: 0x00398f38
     * @ghidraAddress PAL: 0x004077c0
     */
    int Type() override {
        return g_nDataAckPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `DataAckPkt`.
     * @ghidraAddress NTSC-U/C: 0x00398f48
     * @ghidraAddress PAL: 0x004077d0
     */
    const char *GetName() const override {
        return "DataAckPkt";
    }
};

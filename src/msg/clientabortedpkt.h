#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that ClientAbortedPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0bc4
 */
extern int g_nClientAbortedPktType;

/**
 * Packet a console sends every peer when it leaves the song early.
 *
 * The RTTI records the class as deriving from InternalPacket, and its vtable is at `0x003d44b0`.
 * Delivery is guaranteed. The class adds no member, and its payload routines are those of Packet.
 * The members Clone(), Type(), and GetName() are inline.
 */
class ClientAbortedPkt : public InternalPacket {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00398f58
     * @ghidraAddress PAL: 0x004077e0
     */
    ~ClientAbortedPkt() override = default;

    /**
     * Construct a packet for every peer.
     *
     * Inline. New() expands it.
     */
    ClientAbortedPkt() : InternalPacket(kFlagGuaranteed, kAllPeers) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x002679b0
     * @ghidraAddress PAL: 0x00271688
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ClientAbortedPkt", 0);
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
     * @ghidraAddress NTSC-U/C: 0x00398fd8
     * @ghidraAddress PAL: 0x00407860
     */
    Message *Clone() override {
        return new ClientAbortedPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nClientAbortedPktType.
     * @ghidraAddress NTSC-U/C: 0x00399040
     * @ghidraAddress PAL: 0x004078c8
     */
    int Type() override {
        return g_nClientAbortedPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `ClientAbortedPkt`.
     * @ghidraAddress NTSC-U/C: 0x00399050
     * @ghidraAddress PAL: 0x004078d8
     */
    const char *GetName() const override {
        return "ClientAbortedPkt";
    }
};

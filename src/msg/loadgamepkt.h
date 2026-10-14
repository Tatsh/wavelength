#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that LoadGamePkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0bbc
 */
extern int g_nLoadGamePktType;

/**
 * Packet the host sends every console to load the song of the session.
 *
 * The RTTI records the class as deriving from InternalPacket, and its vtable is at `0x003d48b8`.
 * Delivery is guaranteed. The class adds no member, and its payload routines are those of Packet.
 * The members Clone(), Type(), and GetName() are inline.
 */
class LoadGamePkt : public InternalPacket {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039a6e0
     * @ghidraAddress PAL: 0x00408f68
     */
    ~LoadGamePkt() override = default;

    /**
     * Construct a packet for every peer.
     *
     * Inline. New() expands it.
     */
    LoadGamePkt() : InternalPacket(kFlagGuaranteed, kAllPeers) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x002678f8
     * @ghidraAddress PAL: 0x002715d0
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "LoadGamePkt", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0039a760
     * @ghidraAddress PAL: 0x00408fe8
     */
    Message *Clone() override {
        return new LoadGamePkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nLoadGamePktType.
     * @ghidraAddress NTSC-U/C: 0x0039a7c8
     * @ghidraAddress PAL: 0x00409050
     */
    int Type() override {
        return g_nLoadGamePktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `LoadGamePkt`.
     * @ghidraAddress NTSC-U/C: 0x0039a7d8
     * @ghidraAddress PAL: 0x00409060
     */
    const char *GetName() const override {
        return "LoadGamePkt";
    }
};

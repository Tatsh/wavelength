#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that BootPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0bac
 */
extern int g_nBootPktType;

/**
 * Packet the host sends every console when it removes a guest from the session.
 *
 * The RTTI records the class as deriving from InternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline. The member names are inferred.
 */
class BootPkt : public InternalPacket {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003a0f18
     * @ghidraAddress PAL: 0x0040fbf8
     */
    ~BootPkt() override = default;

    /**
     * Construct a packet for every peer with no guest.
     *
     * Inline. New() expands it.
     */
    BootPkt() : InternalPacket(kFlagGuaranteed, kAllPeers), mPlayer(kUnset) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267790
     * @ghidraAddress PAL: 0x00271468
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "BootPkt", 0);
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
     */
    Message *Clone() override {
        return new BootPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nBootPktType.
     */
    int Type() override {
        return g_nBootPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `BootPkt`.
     */
    const char *GetName() const override {
        return "BootPkt";
    }

    /**
     * Write the sender, the receiver, and the guest.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269950
     * @ghidraAddress PAL: 0x00273508
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the guest as four bytes.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269990
     * @ghidraAddress PAL: 0x00273548
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the guest.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x002699c0
     * @ghidraAddress PAL: 0x00273578
     */
    void restoreGuts(BinStream &stream) override;

    int mPlayer; /*!< The identifier in the session of the guest removed, or kUnset. */
};

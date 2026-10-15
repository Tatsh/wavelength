#pragma once

#include "game/remixinfo.h"
#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that ShareRemixPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0bb0
 */
extern int g_nShareRemixPktType;

/**
 * Packet the host sends to offer the remix of the game to the guests.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class ShareRemixPkt : public InternalPacket {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003a1048
     * @ghidraAddress PAL: 0x0040fd28
     */
    ~ShareRemixPkt() override = default;

    /**
     * Construct a packet with a reset description. mShareOnly is left unset.
     *
     * Inline. New() expands it.
     */
    ShareRemixPkt() = default;

    /**
     * Construct a packet for every peer, ordered and guaranteed.
     *
     * Inline.
     *
     * @param info The description of the remix.
     * @param nShareOnly Non-zero when the remix is only shared, not played.
     */
    ShareRemixPkt(const RemixInfo &info, int nShareOnly)
        : InternalPacket(kFlagOrdered | kFlagGuaranteed, kAllPeers), mInfo(info),
          mShareOnly(nShareOnly) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x002677e8
     * @ghidraAddress PAL: 0x002714c0
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ShareRemixPkt", 0);
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
        return new ShareRemixPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nShareRemixPktType.
     */
    int Type() override {
        return g_nShareRemixPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `ShareRemixPkt`.
     */
    const char *GetName() const override {
        return "ShareRemixPkt";
    }

    /**
     * Write the sender, the receiver, the description, and mShareOnly.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002699e8
     * @ghidraAddress PAL: 0x002735a0
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the description and mShareOnly as a byte.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269a40
     * @ghidraAddress PAL: 0x002735f8
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read what saveGuts() wrote, and mark the remix as received over the network.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269a90
     * @ghidraAddress PAL: 0x00273640
     */
    void restoreGuts(BinStream &stream) override;

    RemixInfo mInfo; /*!< The description of the remix. */
    int mShareOnly;  /*!< Non-zero when the remix is only shared, not played. */
};

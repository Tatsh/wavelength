#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that TimeResponsePkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ba4
 */
extern int g_nTimeResponsePktType;

/**
 * Packet that answers a TimeRequestPkt with the clock of the answering console.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class TimeResponsePkt : public InternalPacket {
public:
    /**
     * Construct a packet with the times left unset.
     *
     * Inline. New() expands it.
     */
    TimeResponsePkt() = default;

    /**
     * Construct a packet for one peer, with no delivery flag.
     *
     * Inline.
     *
     * @param nReceiver The peer the packet is for.
     * @param fSendTime The clock of this console now.
     * @param fRequestTime The time of the request the packet answers.
     */
    TimeResponsePkt(int nReceiver, float fSendTime, float fRequestTime)
        : InternalPacket(0, nReceiver), mSendTime(fSendTime), mRequestTime(fRequestTime) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267688
     * @ghidraAddress PAL: 0x00271360
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "TimeResponsePkt", 0);
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
        return new TimeResponsePkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nTimeResponsePktType.
     */
    int Type() override {
        return g_nTimeResponsePktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `TimeResponsePkt`.
     */
    const char *GetName() const override {
        return "TimeResponsePkt";
    }

    /**
     * Write the sender, the receiver, and the two times as ` req:` and ` send:`.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002697a8
     * @ghidraAddress PAL: 0x00273360
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write mRequestTime and then mSendTime.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269810
     * @ghidraAddress PAL: 0x002733c8
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read what saveGuts() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269870
     * @ghidraAddress PAL: 0x00273428
     */
    void restoreGuts(BinStream &stream) override;

    float mSendTime;    /*!< The clock of the answering console when it sent the packet. */
    float mRequestTime; /*!< The time of the request the packet answers. */
};

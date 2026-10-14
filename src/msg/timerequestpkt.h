#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that TimeRequestPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ba0
 */
extern int g_nTimeRequestPktType;

/**
 * Packet that asks a console for its clock, to measure the delay to it.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class TimeRequestPkt : public InternalPacket {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003a0ab8
     * @ghidraAddress PAL: 0x0040f798
     */
    ~TimeRequestPkt() override = default;

    /**
     * Construct a packet with mSendTime left unset.
     *
     * Inline. New() expands it.
     */
    TimeRequestPkt() = default;

    /**
     * Construct a packet for one peer, with no delivery flag.
     *
     * Inline.
     *
     * @param nReceiver The peer the packet is for.
     * @param fSendTime The clock of this console now.
     */
    TimeRequestPkt(int nReceiver, float fSendTime)
        : InternalPacket(0, nReceiver), mSendTime(fSendTime) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267640
     * @ghidraAddress PAL: 0x00271318
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "TimeRequestPkt", 0);
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
        return new TimeRequestPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nTimeRequestPktType.
     */
    int Type() override {
        return g_nTimeRequestPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `TimeRequestPkt`.
     */
    const char *GetName() const override {
        return "TimeRequestPkt";
    }

    /**
     * Write the sender, the receiver, and the time of the request.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269710
     * @ghidraAddress PAL: 0x002732c8
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the time of the request.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269750
     * @ghidraAddress PAL: 0x00273308
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the time of the request.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269780
     */
    void restoreGuts(BinStream &stream) override;

    float mSendTime; /*!< The clock of the requesting console when it sent the packet. */
};

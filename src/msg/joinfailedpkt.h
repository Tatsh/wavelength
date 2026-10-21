#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that JoinFailedPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b90
 * @ghidraAddress PAL: 0x0042fa60
 */
extern int g_nJoinFailedPktType;

/**
 * Packet the host sends a console it turns away.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class JoinFailedPkt : public InternalPacket {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003a0448
     * @ghidraAddress PAL: 0x0040f128
     */
    ~JoinFailedPkt() override = default;

    /**
     * Construct a packet with mReason left unset.
     *
     * Inline. New() expands it.
     */
    JoinFailedPkt() = default;

    /**
     * Construct a packet for one peer, ordered and guaranteed.
     *
     * Inline.
     *
     * @param nReceiver The peer the packet is for.
     * @param nReason Why the console was turned away, a LaunchpadAbortedMsg::Reason.
     */
    JoinFailedPkt(int nReceiver, int nReason)
        : InternalPacket(kFlagOrdered | kFlagGuaranteed, nReceiver), mReason(nReason) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x002674f0
     * @ghidraAddress PAL: 0x002711c8
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "JoinFailedPkt", 0);
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
        return new JoinFailedPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nJoinFailedPktType.
     */
    int Type() override {
        return g_nJoinFailedPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `JoinFailedPkt`.
     */
    const char *GetName() const override {
        return "JoinFailedPkt";
    }

    /**
     * Write the reason alone, without the sender and the receiver.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002693b8
     * @ghidraAddress PAL: 0x00272fa0
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the reason as four bytes.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002693e0
     * @ghidraAddress PAL: 0x00272fc8
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the reason as one signed byte, although saveGuts() writes four.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269410
     * @ghidraAddress PAL: 0x00272ff8
     */
    void restoreGuts(BinStream &stream) override;

    /** @return Why the console was turned away, a LaunchpadAbortedMsg::Reason. */
    int GetReason() const {
        return mReason;
    }

private:
    int mReason; /*!< Why the console was turned away, a LaunchpadAbortedMsg::Reason. */
};

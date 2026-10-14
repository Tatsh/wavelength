#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that ClientStatusPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b98
 */
extern int g_nClientStatusPktType;

/**
 * Packet that reports the state of a console of the session.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class ClientStatusPkt : public InternalPacket {
public:
    /**
     * Construct a packet with mReady left unset.
     *
     * Inline. New() expands it.
     */
    ClientStatusPkt() = default;

    /**
     * Construct a packet for every peer, ordered and guaranteed.
     *
     * Inline.
     *
     * @param nReady The state, as NetLaunchpadPlayer::mReady.
     */
    explicit ClientStatusPkt(int nReady)
        : InternalPacket(kFlagOrdered | kFlagGuaranteed, kAllPeers), mReady(nReady) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267598
     * @ghidraAddress PAL: 0x00271270
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ClientStatusPkt", 0);
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
        return new ClientStatusPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nClientStatusPktType.
     */
    int Type() override {
        return g_nClientStatusPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `ClientStatusPkt`.
     */
    const char *GetName() const override {
        return "ClientStatusPkt";
    }

    /**
     * Write the sender, the receiver, and the state.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002694e8
     * @ghidraAddress PAL: 0x002730c0
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the state as a byte.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269528
     * @ghidraAddress PAL: 0x00273100
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the state as a signed byte.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269568
     */
    void restoreGuts(BinStream &stream) override;

    int mReady; /*!< The state, as NetLaunchpadPlayer::mReady. */
};

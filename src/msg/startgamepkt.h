#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that StartGamePkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0bc0
 */
extern int g_nStartGamePktType;

/**
 * Packet the host sends every console to start the song at an agreed time.
 *
 * The RTTI records the class as deriving from InternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline. The member names are inferred.
 */
class StartGamePkt : public InternalPacket {
public:
    /**
     * Construct a packet for every peer with no time and no seed.
     *
     * Inline. New() expands it.
     */
    StartGamePkt() : InternalPacket(kFlagGuaranteed, kAllPeers), mTime(0.0f), mSeed(0) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267950
     * @ghidraAddress PAL: 0x00271628
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "StartGamePkt", 0);
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
        return new StartGamePkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nStartGamePktType.
     */
    int Type() override {
        return g_nStartGamePktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `StartGamePkt`.
     */
    const char *GetName() const override {
        return "StartGamePkt";
    }

    /**
     * Write the sender, the receiver, the time, and the seed.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269c90
     * @ghidraAddress PAL: 0x00273820
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the time and the seed.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269ce8
     * @ghidraAddress PAL: 0x00273878
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the time and the seed.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269d48
     * @ghidraAddress PAL: 0x002738d8
     */
    void restoreGuts(BinStream &stream) override;

    float mTime; /*!< The clock of the host the song starts at. */
    int mSeed;   /*!< The seed of the random numbers every console's logic draws. */
};

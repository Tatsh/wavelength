#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that FinalScorePacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af740
 */
extern int g_nFinalScorePacketType;

/**
 * Packet that reports a player's score at the end of a song to the hosting console.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed, and the
 * packet is addressed to peer 0. The members Clone(), Type(), and GetName() are inline.
 */
class FinalScorePacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no score.
     *
     * Inline. New() expands it.
     */
    FinalScorePacket() : ExternalPacket(kFlagGuaranteed, 0), mScore(-1) {
    }

    /**
     * Construct a packet with a score.
     *
     * Inline. MultiGameLogic::OnFinish() expands it on its stack.
     *
     * @param nScore The player's score.
     */
    explicit FinalScorePacket(int nScore) : ExternalPacket(kFlagGuaranteed, 0), mScore(nScore) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00123200
     * @ghidraAddress PAL: 0x00124980
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "FinalScorePacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x00335a48
     */
    Message *Clone() override {
        return new FinalScorePacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nFinalScorePacketType.
     * @ghidraAddress NTSC-U/C: 0x00335ab8
     */
    int Type() override {
        return g_nFinalScorePacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `FinalScorePacket`.
     * @ghidraAddress NTSC-U/C: 0x00335ac8
     */
    const char *GetName() const override {
        return "FinalScorePacket";
    }

    /**
     * Write the score.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0010c460
     * @ghidraAddress PAL: 0x0010db98
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the score.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0010c490
     */
    void restoreGuts(BinStream &stream) override;

    int mScore; /*!< The player's score. */
};

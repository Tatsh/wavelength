#pragma once

#include <vector>

#include "game/netgamescore.h"
#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that GameOverPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0bc8
 */
extern int g_nGameOverPktType;

/**
 * Packet that reports the final scores of the song.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class GameOverPkt : public InternalPacket {
public:
    /**
     * Construct a packet with no scores.
     *
     * Inline. New() expands it.
     */
    GameOverPkt() = default;

    /**
     * Construct a packet for every peer, guaranteed.
     *
     * Inline.
     *
     * @param scores The final score of each player.
     */
    explicit GameOverPkt(const std::vector<NetGameScore> &scores)
        : InternalPacket(kFlagGuaranteed, kAllPeers), mScores(scores) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267a08
     * @ghidraAddress PAL: 0x002716e0
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "GameOverPkt", 0);
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
        return new GameOverPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nGameOverPktType.
     */
    int Type() override {
        return g_nGameOverPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `GameOverPkt`.
     */
    const char *GetName() const override {
        return "GameOverPkt";
    }

    /**
     * Write the sender, the receiver, and the scores.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269d98
     * @ghidraAddress PAL: 0x00273928
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the scores.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269dd8
     * @ghidraAddress PAL: 0x00273968
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the scores.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269e00
     * @ghidraAddress PAL: 0x00273990
     */
    void restoreGuts(BinStream &stream) override;

    std::vector<NetGameScore> mScores; /*!< The final score of each player. */
};

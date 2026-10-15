#pragma once

#include <vector>

#include "msg/internalpacket.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"

/**
 * Identity that UpdateRanksPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0bcc
 */
extern int g_nUpdateRanksPktType;

/**
 * Packet that reports the ranks of the players of the song.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class UpdateRanksPkt : public InternalPacket {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003a15f8
     * @ghidraAddress PAL: 0x004102d8
     */
    ~UpdateRanksPkt() override = default;

    /** The rank of one player. The RTTI records the name `UpdateRanksPkt::Info`. */
    struct Info {
        int mPlayer; /*!< The player's identifier in the session. */
        int mRank;   /*!< The player's rank. */
    };

    /**
     * Construct a packet with no ranks.
     *
     * Inline. New() expands it.
     */
    UpdateRanksPkt() = default;

    /**
     * Construct a packet for every peer, ordered and guaranteed.
     *
     * Inline.
     *
     * @param ranks The rank of each player.
     */
    explicit UpdateRanksPkt(const std::vector<Info> &ranks)
        : InternalPacket(kFlagOrdered | kFlagGuaranteed, kAllPeers), mRanks(ranks) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267a60
     * @ghidraAddress PAL: 0x00271738
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "UpdateRanksPkt", 0);
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
        return new UpdateRanksPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nUpdateRanksPktType.
     */
    int Type() override {
        return g_nUpdateRanksPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `UpdateRanksPkt`.
     */
    const char *GetName() const override {
        return "UpdateRanksPkt";
    }

    /**
     * Write the sender, the receiver, and the ranks.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269e28
     * @ghidraAddress PAL: 0x002739b8
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the ranks.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269e68
     * @ghidraAddress PAL: 0x002739f8
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the ranks.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269e90
     * @ghidraAddress PAL: 0x00273a20
     */
    void restoreGuts(BinStream &stream) override;

    std::vector<Info> mRanks; /*!< The rank of each player. */
};

/**
 * Write a rank as `player:rank`.
 *
 * @param stream The stream.
 * @param info The rank.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002687b0
 * @ghidraAddress PAL: 0x00272408
 */
PrnStream &operator<<(PrnStream &stream, const UpdateRanksPkt::Info &info);

/**
 * Write a rank to the packets of a session.
 *
 * @param stream The stream.
 * @param info The rank.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00268808
 * @ghidraAddress PAL: 0x00272460
 */
BinStream &operator<<(BinStream &stream, const UpdateRanksPkt::Info &info);

/**
 * Read a rank operator<<() wrote.
 *
 * @param stream The stream.
 * @param info Receives the rank.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00268868
 * @ghidraAddress PAL: 0x002724c0
 */
BinStream &operator>>(BinStream &stream, UpdateRanksPkt::Info &info);

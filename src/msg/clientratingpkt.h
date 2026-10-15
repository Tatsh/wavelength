#pragma once

#include "msg/internalpacket.h"
#include "netflow/ratingrecord.h"
#include "os/mem.h"

/**
 * Identity that ClientRatingPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b9c
 * @ghidraAddress PAL: 0x0042fa6c
 */
extern int g_nClientRatingPktType;

/**
 * Packet that delivers the new rating of a player after an online game.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class ClientRatingPkt : public InternalPacket {
public:
    /**
     * Construct a packet with an empty record. The other members are left unset.
     *
     * Inline. New() expands it.
     */
    ClientRatingPkt() : mRecord() {
    }

    /**
     * Construct a packet for every peer, ordered and guaranteed.
     *
     * Inline.
     *
     * @param nRating The rating, as LobbyPlayer::mRating.
     * @param nRank The position in the ranking.
     * @param nGames The number of online games played.
     * @param record The results the next rating weighs.
     */
    ClientRatingPkt(int nRating, int nRank, int nGames, const RatingRecord &record)
        : InternalPacket(kFlagOrdered | kFlagGuaranteed, kAllPeers), mRating(nRating), mRank(nRank),
          mGames(nGames), mRecord(record) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x002675e0
     * @ghidraAddress PAL: 0x002712b8
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ClientRatingPkt", 0);
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
        return new ClientRatingPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nClientRatingPktType.
     */
    int Type() override {
        return g_nClientRatingPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `ClientRatingPkt`.
     */
    const char *GetName() const override {
        return "ClientRatingPkt";
    }

    /**
     * Write the sender, the receiver, the rating, the rank, the games, and the record.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002695b0
     * @ghidraAddress PAL: 0x00273168
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the rating, the rank, and the games as four bytes each, and the record.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269628
     * @ghidraAddress PAL: 0x002731e0
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read what saveGuts() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x002696a8
     * @ghidraAddress PAL: 0x00273260
     */
    void restoreGuts(BinStream &stream) override;

    int mRating;          /*!< The rating, as LobbyPlayer::mRating. */
    int mRank;            /*!< The position in the ranking. */
    int mGames;           /*!< The number of online games played. */
    RatingRecord mRecord; /*!< The results the next rating weighs. */
};

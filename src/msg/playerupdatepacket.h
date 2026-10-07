#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that PlayerUpdatePacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af71c
 */
extern int g_nPlayerUpdatePacketType;

/**
 * Packet that reports a local player's track, score, and catching state to the other consoles.
 *
 * The RTTI records the class as deriving from ExternalPacket. LocalPlayer::Broadcast() sends one
 * in the game and duel rule sets, and each one includes the next value of
 * LocalPlayer::sPlayerUpdateVersion. The members Clone(), Type(), and GetName() are inline.
 */
class PlayerUpdatePacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no track and no score.
     *
     * Inline. New() expands it.
     */
    PlayerUpdatePacket() : mTrack(-1), mPoints(-1), mCatching(false), mVersion(0) {
    }

    /**
     * Construct a packet with every field set.
     *
     * Inline. LocalPlayer::Broadcast() expands it on its stack.
     *
     * @param nTrack The index of the player's track.
     * @param nPoints The player's score.
     * @param bCatching The player's catching state.
     * @param nVersion The update's sequence number.
     */
    PlayerUpdatePacket(int nTrack, int nPoints, bool bCatching, int nVersion)
        : mTrack(nTrack), mPoints(nPoints), mCatching(bCatching), mVersion(nVersion) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00122eb8
     * @ghidraAddress PAL: 0x00124638
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "PlayerUpdatePacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0033dd70
     */
    Message *Clone() override {
        return new PlayerUpdatePacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nPlayerUpdatePacketType.
     * @ghidraAddress NTSC-U/C: 0x0033ddf8
     */
    int Type() override {
        return g_nPlayerUpdatePacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `PlayerUpdatePacket`.
     * @ghidraAddress NTSC-U/C: 0x0033de08
     */
    const char *GetName() const override {
        return "PlayerUpdatePacket";
    }

    /**
     * Print the track, the score, the catching state, and the version.
     *
     * @param stream The stream to print to.
     * @ghidraAddress NTSC-U/C: 0x0012e2c8
     * @ghidraAddress PAL: 0x0012faa0
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the fields packed into one word.
     *
     * The track takes the low three bits, the score the next fourteen, the catching state one bit,
     * and the version the next fourteen.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0012e1b0
     * @ghidraAddress PAL: 0x0012f988
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the word saveGuts() writes back into the fields.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0012e240
     * @ghidraAddress PAL: 0x0012fa18
     */
    void restoreGuts(BinStream &stream) override;

    int mTrack;     /*!< The index of the player's track. */
    int mPoints;    /*!< The player's score. */
    bool mCatching; /*!< The player's catching state. */
    int mVersion;   /*!< The update's sequence number. */
};

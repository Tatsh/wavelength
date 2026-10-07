#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that RemixerUpdatePacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af744
 */
extern int g_nRemixerUpdatePacketType;

/**
 * Packet that reports the track and repeat state of a local player in a remix to the other
 * consoles.
 *
 * The RTTI records the class as deriving from ExternalPacket. LocalPlayer::Broadcast() sends one
 * in the remix rule set, and each one includes the next value of
 * LocalPlayer::sRemixerUpdateVersion. The members Clone(), Type(), and GetName() are inline.
 */
class RemixerUpdatePacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no track.
     *
     * Inline. New() expands it.
     */
    RemixerUpdatePacket() : mTrack(-1), mRepeat(false), mVersion(0) {
    }

    /**
     * Construct a packet with every field set.
     *
     * Inline. LocalPlayer::Broadcast() expands it on its stack.
     *
     * @param nTrack The index of the player's track.
     * @param bRepeat The player's repeat state.
     * @param nVersion The update's sequence number.
     */
    RemixerUpdatePacket(int nTrack, bool bRepeat, int nVersion)
        : mTrack(nTrack), mRepeat(bRepeat), mVersion(nVersion) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00123258
     * @ghidraAddress PAL: 0x001249d8
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "RemixerUpdatePacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0033e168
     */
    Message *Clone() override {
        return new RemixerUpdatePacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nRemixerUpdatePacketType.
     * @ghidraAddress NTSC-U/C: 0x0033e1e8
     */
    int Type() override {
        return g_nRemixerUpdatePacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `RemixerUpdatePacket`.
     * @ghidraAddress NTSC-U/C: 0x0033e1f8
     */
    const char *GetName() const override {
        return "RemixerUpdatePacket";
    }

    /**
     * Print the track, the repeat state, and the version.
     *
     * @param stream The stream to print to.
     * @ghidraAddress NTSC-U/C: 0x0012fb80
     * @ghidraAddress PAL: 0x001313a8
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the fields packed into one word.
     *
     * The track takes the low three bits, the repeat state one bit, and the version the next 28.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0012fa88
     * @ghidraAddress PAL: 0x001312b0
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the word saveGuts() writes back into the fields.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0012fb08
     * @ghidraAddress PAL: 0x00131330
     */
    void restoreGuts(BinStream &stream) override;

    int mTrack;   /*!< The index of the player's track. */
    bool mRepeat; /*!< The player's repeat state. */
    int mVersion; /*!< The update's sequence number. */
};

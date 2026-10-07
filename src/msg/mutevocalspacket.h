#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that MuteVocalsPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af750
 */
extern int g_nMuteVocalsPacketType;

/**
 * Packet that reports the vocals of a track of a remix section muted or restored.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline.
 */
class MuteVocalsPacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no section and no track.
     *
     * Inline. New() expands it.
     */
    MuteVocalsPacket() : ExternalPacket(kFlagGuaranteed), mSection(-1), mTrack(-1), mMute(false) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00123378
     * @ghidraAddress PAL: 0x00124af8
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "MuteVocalsPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0033c608
     * @ghidraAddress PAL: 0x003a9b40
     */
    Message *Clone() override {
        return new MuteVocalsPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nMuteVocalsPacketType.
     * @ghidraAddress NTSC-U/C: 0x0033c688
     * @ghidraAddress PAL: 0x003a9bc0
     */
    int Type() override {
        return g_nMuteVocalsPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `MuteVocalsPacket`.
     * @ghidraAddress NTSC-U/C: 0x0033c698
     * @ghidraAddress PAL: 0x003a9bd0
     */
    const char *GetName() const override {
        return "MuteVocalsPacket";
    }

    /**
     * Write the track and the mute state to a diagnostic stream.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00127a38
     * @ghidraAddress PAL: 0x00129230
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the section, the track, and the mute state packed into one byte.
     *
     * The section takes the low four bits, the track the next three, and the mute state the top
     * bit.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00127918
     * @ghidraAddress PAL: 0x00129128
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the byte saveGuts() writes back into the fields.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x001279b0
     * @ghidraAddress PAL: 0x001291b8
     */
    void restoreGuts(BinStream &stream) override;

    int mSection; /*!< The section. */
    int mTrack;   /*!< The track. */
    bool mMute;   /*!< Whether the vocals are muted. */
};

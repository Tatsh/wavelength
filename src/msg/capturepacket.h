#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that CapturePacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af724
 */
extern int g_nCapturePacketType;

/**
 * Packet that reports a phrase of a track captured by a player on another console.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline. The member names are inferred.
 */
class CapturePacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no track and no bar.
     *
     * Inline. New() expands it.
     */
    CapturePacket()
        : ExternalPacket(kFlagGuaranteed), mTrack(-1), mBar(-1), mAutocatch(false), mValid(false) {
    }

    /**
     * Construct a packet with every field set.
     *
     * Inline. MultiGameLogic::OnPhraseCaptured() expands it on its stack.
     *
     * @param nTrack The index of the track.
     * @param nBar The bar.
     * @param bAutocatch Whether an autocatcher captured the phrase.
     */
    CapturePacket(int nTrack, int nBar, bool bAutocatch)
        : ExternalPacket(kFlagGuaranteed), mTrack(nTrack), mBar(nBar), mAutocatch(bAutocatch),
          mValid(true) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00122f78
     * @ghidraAddress PAL: 0x001246f8
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "CapturePacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x00349f00
     * @ghidraAddress PAL: 0x003b7330
     */
    Message *Clone() override {
        return new CapturePacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nCapturePacketType.
     * @ghidraAddress NTSC-U/C: 0x00349f88
     * @ghidraAddress PAL: 0x003b73b8
     */
    int Type() override {
        return g_nCapturePacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `CapturePacket`.
     * @ghidraAddress NTSC-U/C: 0x00349f98
     * @ghidraAddress PAL: 0x003b73c8
     */
    const char *GetName() const override {
        return "CapturePacket";
    }

    /**
     * Write the fields packed into one half-word.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0014b208
     * @ghidraAddress PAL: 0x0014cba8
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the half-word saveGuts() writes back into the fields.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0014b2a0
     * @ghidraAddress PAL: 0x0014cc40
     */
    void restoreGuts(BinStream &stream) override;

    int mTrack;      /*!< The index of the track. */
    int mBar;        /*!< The bar of the capture. */
    bool mAutocatch; /*!< Whether an autocatcher captured the phrase. */
    bool mValid;     /*!< Whether a sender filled the packet. */
};

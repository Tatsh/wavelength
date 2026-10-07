#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that EraseBarPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af758
 */
extern int g_nEraseBarPacketType;

/**
 * Packet that reports a bar of a track erased in the remix editor.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline.
 */
class EraseBarPacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no track and no bar.
     *
     * Inline. New() expands it.
     */
    EraseBarPacket() : ExternalPacket(kFlagGuaranteed), mTrack(-1), mBar(-1) {
    }

    /**
     * Construct a packet for an erased bar.
     *
     * Inline. PitchTrack::HandleInput() expands it.
     *
     * @param nTrack The track.
     * @param nBar The bar.
     */
    EraseBarPacket(int nTrack, int nBar)
        : ExternalPacket(kFlagGuaranteed), mTrack(nTrack), mBar(nBar) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00123440
     * @ghidraAddress PAL: 0x00124bc0
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "EraseBarPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0034b1f8
     * @ghidraAddress PAL: 0x003b8628
     */
    Message *Clone() override {
        return new EraseBarPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nEraseBarPacketType.
     * @ghidraAddress NTSC-U/C: 0x0034b270
     * @ghidraAddress PAL: 0x003b86a0
     */
    int Type() override {
        return g_nEraseBarPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `EraseBarPacket`.
     * @ghidraAddress NTSC-U/C: 0x0034b280
     * @ghidraAddress PAL: 0x003b86b0
     */
    const char *GetName() const override {
        return "EraseBarPacket";
    }

    int mTrack; /*!< The track. */
    int mBar;   /*!< The bar erased. */
};

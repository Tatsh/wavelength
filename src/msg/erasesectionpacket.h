#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that EraseSectionPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af75c
 */
extern int g_nEraseSectionPacketType;

/**
 * Packet that reports a section of a track erased in the remix editor.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline.
 */
class EraseSectionPacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no track and no section.
     *
     * Inline. New() expands it.
     */
    EraseSectionPacket() : ExternalPacket(kFlagGuaranteed), mTrack(-1), mBar(-1) {
    }

    /**
     * Construct a packet for an erased section.
     *
     * Inline. PitchTrack::HandleInput() expands it.
     *
     * @param nTrack The track.
     * @param nBar A bar of the section.
     */
    EraseSectionPacket(int nTrack, int nBar)
        : ExternalPacket(kFlagGuaranteed), mTrack(nTrack), mBar(nBar) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x001234a0
     * @ghidraAddress PAL: 0x00124c20
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "EraseSectionPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0034b340
     * @ghidraAddress PAL: 0x003b8770
     */
    Message *Clone() override {
        return new EraseSectionPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nEraseSectionPacketType.
     * @ghidraAddress NTSC-U/C: 0x0034b3b8
     * @ghidraAddress PAL: 0x003b87e8
     */
    int Type() override {
        return g_nEraseSectionPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `EraseSectionPacket`.
     * @ghidraAddress NTSC-U/C: 0x0034b3c8
     */
    const char *GetName() const override {
        return "EraseSectionPacket";
    }

    int mTrack; /*!< The track. */
    int mBar;   /*!< A bar of the section erased. */
};

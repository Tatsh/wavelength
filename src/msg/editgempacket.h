#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that EditGemPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af754
 */
extern int g_nEditGemPacketType;

/**
 * Packet that reports a gem placed or removed in the remix editor.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline. The member names are inferred.
 */
class EditGemPacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no track and no gem.
     *
     * Inline. New() expands it.
     */
    EditGemPacket()
        : ExternalPacket(kFlagGuaranteed), mTrack(-1), mSlot(-1), mTick(-1), mRepeat(false) {
    }

    /**
     * Construct a packet for a gem placed or removed.
     *
     * Inline. PitchTrack::HandleInput() expands it.
     *
     * @param nTrack The track.
     * @param nSlot The gem button of the gem.
     * @param nTick The tick of the gem.
     * @param bRepeat Whether the edit repeats on every other bar of the section.
     */
    EditGemPacket(int nTrack, int nSlot, int nTick, bool bRepeat)
        : ExternalPacket(kFlagGuaranteed), mTrack(nTrack), mSlot(nSlot), mTick(nTick),
          mRepeat(bRepeat) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x001233d8
     * @ghidraAddress PAL: 0x00124b58
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "EditGemPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x003358a8
     */
    Message *Clone() override {
        return new EditGemPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nEditGemPacketType.
     * @ghidraAddress NTSC-U/C: 0x00335930
     */
    int Type() override {
        return g_nEditGemPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `EditGemPacket`.
     * @ghidraAddress NTSC-U/C: 0x00335940
     */
    const char *GetName() const override {
        return "EditGemPacket";
    }

    int mTrack;   /*!< The track. */
    int mSlot;    /*!< The gem button of the gem. */
    int mTick;    /*!< The tick of the gem. */
    bool mRepeat; /*!< Whether the edit repeats on every other bar of the section. */
};

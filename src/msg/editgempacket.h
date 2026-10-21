#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that EditGemPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af754
 * @ghidraAddress PAL: 0x0042e5ec
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
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00335808
     * @ghidraAddress PAL: 0x003a2db8
     */
    ~EditGemPacket() override = default;

    /**
     * Construct a packet with no track and no gem.
     *
     * Inline. New() expands it.
     */
    EditGemPacket()
        : ExternalPacket(kFlagGuaranteed), mTrack(-1), mSlot(-1), mTick(-1), mRepeat(0) {
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
     * @ghidraAddress PAL: 0x003a2e58
     */
    Message *Clone() override {
        return new EditGemPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nEditGemPacketType.
     * @ghidraAddress NTSC-U/C: 0x00335930
     * @ghidraAddress PAL: 0x003a2ee0
     */
    int Type() override {
        return g_nEditGemPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `EditGemPacket`.
     * @ghidraAddress NTSC-U/C: 0x00335940
     * @ghidraAddress PAL: 0x003a2ef0
     */
    const char *GetName() const override {
        return "EditGemPacket";
    }

    /**
     * Write the fields packed into one word.
     *
     * The track takes three bits, the gem button two, the tick 26, and the repeat flag one.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0010c348
     * @ghidraAddress PAL: 0x0010da80
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the word saveGuts() writes back into the fields.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0010c3d8
     * @ghidraAddress PAL: 0x0010db10
     */
    void restoreGuts(BinStream &stream) override;

    /** @return Whether the edit repeats on every other bar of the section. */
    int GetRepeat() const {
        return mRepeat;
    }

    /** @return The gem button of the gem. */
    int GetSlot() const {
        return mSlot;
    }

    /** @return The tick of the gem. */
    int GetTick() const {
        return mTick;
    }

    /** @return The track, in a duel the side of the player who catches the gem. */
    int GetTrack() const {
        return mTrack;
    }

private:
    int mTrack;  /*!< The track, in a duel the side of the player who catches the gem. */
    int mSlot;   /*!< The gem button of the gem. */
    int mTick;   /*!< The tick of the gem. */
    int mRepeat; /*!< Whether the edit repeats on every other bar of the section. */
};

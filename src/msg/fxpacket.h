#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that FXPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af74c
 */
extern int g_nFXPacketType;

/**
 * Packet that reports a remix effect switched on or off for a track of a section.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline.
 */
class FXPacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no section and no track.
     *
     * Inline. New() expands it.
     */
    FXPacket() : ExternalPacket(kFlagGuaranteed), mSection(-1), mTrack(-1), mType(0), mOn(false) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00123310
     * @ghidraAddress PAL: 0x00124a90
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "FXPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0034bf90
     */
    Message *Clone() override {
        return new FXPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nFXPacketType.
     * @ghidraAddress NTSC-U/C: 0x0034c018
     */
    int Type() override {
        return g_nFXPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `FXPacket`.
     * @ghidraAddress NTSC-U/C: 0x0034c028
     */
    const char *GetName() const override {
        return "FXPacket";
    }

    /**
     * Write the section, the track, the effect, and the switch to a diagnostic stream.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00150ef0
     * @ghidraAddress PAL: 0x001527e0
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the payload packed into two bytes.
     *
     * The section takes the low four bits, the track the next three, the effect the next two, and
     * the switch the bit after them.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00150dc8
     * @ghidraAddress PAL: 0x001526b8
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the two bytes saveGuts() writes back into the fields.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00150e68
     * @ghidraAddress PAL: 0x00152758
     */
    void restoreGuts(BinStream &stream) override;

    int mSection; /*!< The section. */
    int mTrack;   /*!< The track. */
    int mType;    /*!< The effect. */
    bool mOn;     /*!< Whether the effect is switched on. */
};

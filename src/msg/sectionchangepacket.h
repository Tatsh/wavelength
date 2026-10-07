#pragma once

#include "msg/externalpacket.h"
#include "os/binstream.h"
#include "os/mem.h"

/**
 * Identity that SectionChangePacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af748
 */
extern int g_nSectionChangePacketType;

/**
 * Packet that tells the other consoles of a remix session about a section change.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline. The payload travels as one four-byte word, the section
 * in the low four bits and the tick in the other 28.
 */
class SectionChangePacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no section.
     *
     * Inline. New() expands it.
     */
    SectionChangePacket() : ExternalPacket(kFlagGuaranteed), mSection(-1), mTick(-1) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x001232b0
     * @ghidraAddress PAL: 0x00124a30
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "SectionChangePacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x003409a8
     * @ghidraAddress PAL: 0x003adee0
     */
    Message *Clone() override {
        return new SectionChangePacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nSectionChangePacketType.
     * @ghidraAddress NTSC-U/C: 0x00340a20
     * @ghidraAddress PAL: 0x003adf58
     */
    int Type() override {
        return g_nSectionChangePacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `SectionChangePacket`.
     * @ghidraAddress NTSC-U/C: 0x00340a30
     * @ghidraAddress PAL: 0x003adf68
     */
    const char *GetName() const override {
        return "SectionChangePacket";
    }

    /**
     * Write the payload.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00139058
     * @ghidraAddress PAL: 0x0013a8b8
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the payload.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x001390d0
     * @ghidraAddress PAL: 0x0013a930
     */
    void restoreGuts(BinStream &stream) override;

    int mSection; /*!< The section. Four bits travel. */
    int mTick;    /*!< The tick of the change. 28 bits travel. */
};

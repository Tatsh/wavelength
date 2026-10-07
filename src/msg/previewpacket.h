#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that PreviewPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af760
 */
extern int g_nPreviewPacketType;

/**
 * Packet that reports the remix editor's preview of a section started or stopped.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline. The member names are inferred.
 */
class PreviewPacket : public ExternalPacket {
public:
    /**
     * Construct a packet with no section.
     *
     * Inline. New() expands it. mPreviewing is left unset.
     */
    PreviewPacket() : ExternalPacket(kFlagGuaranteed), mSection(-1) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00123500
     * @ghidraAddress PAL: 0x00124c80
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "PreviewPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0033dfb0
     */
    Message *Clone() override {
        return new PreviewPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nPreviewPacketType.
     * @ghidraAddress NTSC-U/C: 0x0033e028
     */
    int Type() override {
        return g_nPreviewPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `PreviewPacket`.
     * @ghidraAddress NTSC-U/C: 0x0033e038
     */
    const char *GetName() const override {
        return "PreviewPacket";
    }

    /**
     * Write the fields packed into one word.
     *
     * The section takes the low 31 bits and the preview state the top bit.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0012ef20
     * @ghidraAddress PAL: 0x001306f8
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the word saveGuts() writes back into the fields.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0012ef90
     * @ghidraAddress PAL: 0x00130768
     */
    void restoreGuts(BinStream &stream) override;

    int mSection;     /*!< The section previewed. */
    bool mPreviewing; /*!< Whether the preview started rather than stopped. */
};

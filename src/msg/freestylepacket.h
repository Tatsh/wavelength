#pragma once

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that FreestylePacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af738
 */
extern int g_nFreestylePacketType;

/**
 * Packet that reports a freestyle player's button and stick to the other consoles.
 *
 * The RTTI records the class as deriving from ExternalPacket. The members Clone(), Type(), and
 * GetName() are inline.
 */
class FreestylePacket : public ExternalPacket {
public:
    /**
     * Construct an inactive packet.
     *
     * Inline. New() expands it.
     */
    FreestylePacket() : mActive(false), mButton(0), mX(0.0f), mY(0.0f) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00123140
     * @ghidraAddress PAL: 0x001248c0
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "FreestylePacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0034b970
     */
    Message *Clone() override {
        return new FreestylePacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nFreestylePacketType.
     * @ghidraAddress NTSC-U/C: 0x0034b9f8
     */
    int Type() override {
        return g_nFreestylePacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `FreestylePacket`.
     * @ghidraAddress NTSC-U/C: 0x0034ba08
     */
    const char *GetName() const override {
        return "FreestylePacket";
    }

    bool mActive; /*!< Whether the player is playing freestyle. */
    int mButton;  /*!< The button held. */
    float mX;     /*!< The stick's horizontal position. */
    float mY;     /*!< The stick's vertical position. */
};

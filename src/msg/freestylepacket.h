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
     * Construct a packet for a freestyle player.
     *
     * Inline. FreestyleTrack::SendUpdate() expands it on its stack.
     *
     * @param bActive Whether the player is playing freestyle.
     * @param nButton The button held.
     * @param fX The stick's horizontal position.
     * @param fY The stick's vertical position.
     */
    FreestylePacket(bool bActive, int nButton, float fX, float fY)
        : mActive(bActive), mButton(nButton), mX(fX), mY(fY) {
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

    /**
     * Write the state and the button, then each stick position in fixed point, in one half-word.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0014faa0
     * @ghidraAddress PAL: 0x00151400
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the half-word saveGuts() writes back into the fields.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0014fb68
     * @ghidraAddress PAL: 0x001514c8
     */
    void restoreGuts(BinStream &stream) override;

    /**
     * Write the state, the button, and the stick position.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0014fc18
     * @ghidraAddress PAL: 0x00151578
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Convert a value from -1 to 1 to a signed fixed-point value.
     *
     * The name is inferred.
     *
     * @param fValue The value.
     * @param nBits The width of the result, sign included.
     * @return The fixed-point value, rounded.
     * @ghidraAddress NTSC-U/C: 0x0014fa28
     * @ghidraAddress PAL: 0x00151388
     */
    static int Quantize(float fValue, int nBits);

    /**
     * Convert a signed fixed-point value back to a value from -1 to 1.
     *
     * The name is inferred.
     *
     * @param nValue The fixed-point value.
     * @param nBits The width of the value, sign included.
     * @return The value.
     * @ghidraAddress NTSC-U/C: 0x0014fa68
     * @ghidraAddress PAL: 0x001513c8
     */
    static float Dequantize(int nValue, int nBits);

    bool mActive; /*!< Whether the player is playing freestyle. */
    int mButton;  /*!< The button held. */
    float mX;     /*!< The stick's horizontal position. */
    float mY;     /*!< The stick's vertical position. */
};

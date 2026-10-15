#pragma once

#include "msg/internalpacket.h"
#include "netflow/netgameparams.h"
#include "os/mem.h"

/**
 * Identity that WelcomePkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b8c
 * @ghidraAddress PAL: 0x0042fa5c
 */
extern int g_nWelcomePktType;

/**
 * Packet the host sends a console that joins the session, with the colour of the console and the
 * settings of the game.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class WelcomePkt : public InternalPacket {
public:
    /**
     * Construct a packet with the default settings. mColor is left unset.
     *
     * Inline. New() expands it.
     */
    WelcomePkt() = default;

    /**
     * Construct a packet for one peer, ordered and guaranteed.
     *
     * Inline.
     *
     * @param nReceiver The peer the packet is for.
     * @param nColor The colour of the slot of the console, 0 to 3.
     * @param params The settings of the game.
     */
    WelcomePkt(int nReceiver, int nColor, const NetGameParams &params)
        : InternalPacket(kFlagOrdered | kFlagGuaranteed, nReceiver), mColor(nColor),
          mParams(params) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267430
     * @ghidraAddress PAL: 0x00271108
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "WelcomePkt", 0);
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
     */
    Message *Clone() override {
        return new WelcomePkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nWelcomePktType.
     */
    int Type() override {
        return g_nWelcomePktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `WelcomePkt`.
     */
    const char *GetName() const override {
        return "WelcomePkt";
    }

    /**
     * Write the sender, the receiver, the colour, and the settings.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002692a0
     * @ghidraAddress PAL: 0x00272ea8
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the colour as a byte and the settings.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002692f8
     * @ghidraAddress PAL: 0x00272f00
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read what saveGuts() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269358
     * @ghidraAddress PAL: 0x00272f50
     */
    void restoreGuts(BinStream &stream) override;

    int mColor;            /*!< The colour of the slot of the console, 0 to 3. */
    NetGameParams mParams; /*!< The settings of the game. */
};

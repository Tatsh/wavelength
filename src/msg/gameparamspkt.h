#pragma once

#include "msg/internalpacket.h"
#include "netflow/netgameparams.h"
#include "os/mem.h"

/**
 * Identity that GameParamsPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0ba8
 * @ghidraAddress PAL: 0x0042fa78
 */
extern int g_nGameParamsPktType;

/**
 * Packet the host sends when the settings of the game change.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline.
 */
class GameParamsPkt : public InternalPacket {
public:
    /**
     * Construct a packet with the default settings.
     *
     * Inline. New() expands it.
     */
    GameParamsPkt() = default;

    /**
     * Construct a packet for every peer, ordered and guaranteed.
     *
     * Inline.
     *
     * @param params The settings of the game.
     */
    explicit GameParamsPkt(const NetGameParams &params)
        : InternalPacket(kFlagOrdered | kFlagGuaranteed, kAllPeers), mParams(params) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x002676d0
     * @ghidraAddress PAL: 0x002713a8
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "GameParamsPkt", 0);
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
        return new GameParamsPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nGameParamsPktType.
     */
    int Type() override {
        return g_nGameParamsPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `GameParamsPkt`.
     */
    const char *GetName() const override {
        return "GameParamsPkt";
    }

    /**
     * Write the sender, the receiver, and the settings.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002698c0
     * @ghidraAddress PAL: 0x00273478
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the settings.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269900
     * @ghidraAddress PAL: 0x002734b8
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the settings.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269928
     * @ghidraAddress PAL: 0x002734e0
     */
    void restoreGuts(BinStream &stream) override;

    NetGameParams mParams; /*!< The settings of the game. */
};

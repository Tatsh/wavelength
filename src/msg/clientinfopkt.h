#pragma once

#include "msg/internalpacket.h"
#include "netflow/netlaunchpadplayer.h"
#include "os/mem.h"

/**
 * Identity that ClientInfoPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b94
 */
extern int g_nClientInfoPktType;

/**
 * Packet that describes one player of the session.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class ClientInfoPkt : public InternalPacket {
public:
    /**
     * Construct a packet with a default LobbyPlayer. The rest of mPlayer is left unset.
     *
     * Inline. New() expands it.
     */
    ClientInfoPkt() = default;

    /**
     * Construct a packet for one peer, ordered and guaranteed.
     *
     * Inline.
     *
     * @param player The player.
     * @param nReceiver The peer the packet is for.
     */
    ClientInfoPkt(const NetLaunchpadPlayer &player, int nReceiver)
        : InternalPacket(kFlagOrdered | kFlagGuaranteed, nReceiver), mPlayer(player) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00267538
     * @ghidraAddress PAL: 0x00271210
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ClientInfoPkt", 0);
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
        return new ClientInfoPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nClientInfoPktType.
     */
    int Type() override {
        return g_nClientInfoPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `ClientInfoPkt`.
     */
    const char *GetName() const override {
        return "ClientInfoPkt";
    }

    /**
     * Write the sender, the receiver, and the player.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269458
     * @ghidraAddress PAL: 0x00273030
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the player.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269498
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the player.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x002694c0
     */
    void restoreGuts(BinStream &stream) override;

    NetLaunchpadPlayer mPlayer; /*!< The player. */
};

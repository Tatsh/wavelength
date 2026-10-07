#pragma once

#include <vector>

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that BumperPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af728
 */
extern int g_nBumperPacketType;

/**
 * Packet that reports the players a bumper power-up knocked off their tracks.
 *
 * The RTTI records the class as deriving from ExternalPacket. Delivery is guaranteed. The members
 * Clone(), Type(), and GetName() are inline.
 */
class BumperPacket : public ExternalPacket {
public:
    /** One player the bumper struck. */
    struct VictimData {
        int mNetOrder; /*!< The player's session order. */
        int mTrack;    /*!< The track the player was moved to. */
    };

    /**
     * Construct a packet with no victims.
     *
     * Inline. New() expands it.
     */
    BumperPacket() : ExternalPacket(kFlagGuaranteed) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00122fe0
     * @ghidraAddress PAL: 0x00124760
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "BumperPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x00349cd8
     * @ghidraAddress PAL: 0x003b7108
     */
    Message *Clone() override {
        return new BumperPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nBumperPacketType.
     * @ghidraAddress NTSC-U/C: 0x00349d18
     * @ghidraAddress PAL: 0x003b7148
     */
    int Type() override {
        return g_nBumperPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `BumperPacket`.
     * @ghidraAddress NTSC-U/C: 0x00349d28
     * @ghidraAddress PAL: 0x003b7158
     */
    const char *GetName() const override {
        return "BumperPacket";
    }

    std::vector<VictimData> mVictims; /*!< The players struck. */
};

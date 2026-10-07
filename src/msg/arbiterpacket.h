#pragma once

#include <vector>

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that ArbiterPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af720
 */
extern int g_nArbiterPacketType;

/**
 * Packet in which the arbitrating console assigns every player a track.
 *
 * The RTTI records the class as deriving from ExternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred from the packet's printed form.
 */
class ArbiterPacket : public ExternalPacket {
public:
    /** One player's assignment. */
    struct PlayerData {
        int mNetOrder; /*!< The player's session order. */
        int mVersion;  /*!< The sequence number of the player's last update. */
        int mTrack;    /*!< The track assigned to the player. */
        int mSlot;     /*!< The slot assigned to the player. */
    };

    /**
     * Construct a packet with no assignments.
     *
     * Inline. New() expands it.
     */
    ArbiterPacket() : mVersion(-1) {
    }

    /**
     * Construct an empty packet with a sequence number.
     *
     * Inline. NetArbiter::Broadcast() expands it on its stack.
     *
     * @param nVersion The sequence number.
     */
    explicit ArbiterPacket(int nVersion) : mVersion(nVersion) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x00122f18
     * @ghidraAddress PAL: 0x00124698
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ArbiterPacket", 0);
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
     * @ghidraAddress NTSC-U/C: 0x00347b00
     */
    Message *Clone() override {
        return new ArbiterPacket(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nArbiterPacketType.
     * @ghidraAddress NTSC-U/C: 0x00347b40
     */
    int Type() override {
        return g_nArbiterPacketType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `ArbiterPacket`.
     * @ghidraAddress NTSC-U/C: 0x00347b50
     * @ghidraAddress PAL: 0x003b4ec0
     */
    const char *GetName() const override {
        return "ArbiterPacket";
    }

    std::vector<PlayerData> mPlayers; /*!< The assignments. */
    int mVersion;                     /*!< The assignment's sequence number. */
};

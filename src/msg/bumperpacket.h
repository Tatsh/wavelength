#pragma once

#include <vector>

#include "msg/externalpacket.h"
#include "os/mem.h"

/**
 * Identity that BumperPacket::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003af728
 * @ghidraAddress PAL: 0x0042e5c0
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
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00349bc8
     * @ghidraAddress PAL: 0x003b6ff8
     */
    ~BumperPacket() override = default;

    /** One player the bumper struck. */
    class VictimData {
    public:
        /** Construct an entry with every field unset. */
        VictimData() = default;

        /**
         * Construct an entry.
         *
         * @param nNetOrder The player's session order.
         * @param nTrack The track the player was moved to.
         * @ghidraAddress NTSC-U/C: 0x0033c128
         * @ghidraAddress PAL: 0x003a9660
         */
        VictimData(int nNetOrder, int nTrack) : mNetOrder(nNetOrder), mTrack(nTrack) {
        }

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
     * Copy another packet.
     *
     * @param other The packet.
     * @ghidraAddress NTSC-U/C: 0x00349d50
     * @ghidraAddress PAL: 0x003b7180
     */
    BumperPacket(const BumperPacket &other) = default;

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

    /**
     * Write the victims, at most three.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0014a960
     * @ghidraAddress PAL: 0x0014c320
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read the victims saveGuts() writes, after the victims are removed.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0014aa38
     * @ghidraAddress PAL: 0x0014c3f0
     */
    void restoreGuts(BinStream &stream) override;

    /**
     * Write each victim's player and track.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0014ad30
     * @ghidraAddress PAL: 0x0014c6d0
     */
    void PrintExtra(PrnStream &stream) const override;

    /** @return The players struck. */
    const std::vector<VictimData> &GetVictims() const {
        return mVictims;
    }

    /** @return The players struck, for filling. */
    std::vector<VictimData> &GetVictims() {
        return mVictims;
    }

private:
    std::vector<VictimData> mVictims; /*!< The players struck. */
};

#pragma once

#include "msg/internalpacket.h"
#include "os/mem.h"

/**
 * Identity that DataBlockPkt::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0bb8
 * @ghidraAddress PAL: 0x0042fa88
 */
extern int g_nDataBlockPktType;

/**
 * Packet that delivers one block of the remix the host shares.
 *
 * The RTTI records the class as deriving from InternalPacket. The members Clone(), Type(), and
 * GetName() are inline. The member names are inferred.
 */
class DataBlockPkt : public InternalPacket {
public:
    /** The most bytes one block delivers. */
    static constexpr int kMaxBlockSize = 256;

    /**
     * Construct a packet with every member left unset.
     *
     * Inline. New() expands it.
     */
    DataBlockPkt() = default;

    /**
     * Construct a packet for every peer, ordered and guaranteed.
     *
     * Inline. mBuffer is left unset.
     *
     * @param pData The bytes saveGuts() sends.
     * @param nBlock The index of the block.
     * @param nSize The number of bytes of the block.
     * @param fProgress The part of the remix delivered with the block, 0 to 1.
     */
    DataBlockPkt(const char *pData, short nBlock, short nSize, float fProgress)
        : InternalPacket(kFlagOrdered | kFlagGuaranteed, kAllPeers), mData(pData), mBlock(nBlock),
          mSize(nSize), mProgress(fProgress) {
    }

    /**
     * Produce a default-constructed packet on the heap.
     *
     * @return The packet.
     * @ghidraAddress NTSC-U/C: 0x002678b0
     * @ghidraAddress PAL: 0x00271588
     */
    static Message *New();

    /**
     * Allocate a packet, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "DataBlockPkt", 0);
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
        return new DataBlockPkt(*this);
    }

    /**
     * Report this packet's registered identity.
     *
     * @return g_nDataBlockPktType.
     */
    int Type() override {
        return g_nDataBlockPktType;
    }

    /**
     * Report this packet's class name.
     *
     * @return The literal `DataBlockPkt`.
     */
    const char *GetName() const override {
        return "DataBlockPkt";
    }

    /**
     * Write the sender, the receiver, the block, the size, and the progress.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269af0
     * @ghidraAddress PAL: 0x00273690
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Write the block, the size, the progress, and mSize bytes from mData.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269b88
     * @ghidraAddress PAL: 0x00273728
     */
    void saveGuts(BinStream &stream) const override;

    /**
     * Read what saveGuts() wrote, the bytes into mBuffer.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00269c18
     * @ghidraAddress PAL: 0x002737b0
     */
    void restoreGuts(BinStream &stream) override;

    /** @return The index of the block. */
    short GetBlock() const {
        return mBlock;
    }

    /** @return The part of the remix delivered with the block, 0 to 1. */
    float GetProgress() const {
        return mProgress;
    }

    /** @return The number of bytes of the block. */
    short GetSize() const {
        return mSize;
    }

    /** @return The bytes restoreGuts() receives. */
    const char *GetBuffer() const {
        return mBuffer;
    }

private:
    const char *mData;           /*!< The bytes saveGuts() sends. */
    char mBuffer[kMaxBlockSize]; /*!< The bytes restoreGuts() receives. */
    short mBlock;                /*!< The index of the block. */
    short mSize;                 /*!< The number of bytes of the block. */
    float mProgress;             /*!< The part of the remix delivered with the block, 0 to 1. */
};

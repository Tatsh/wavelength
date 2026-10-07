#pragma once

#include <inet.h>

/**
 * Header of one datagram in the ring buffer of an AsyncInfo. The datagram follows the header, and
 * the next header follows the datagram's capacity. The ring buffer has a copy in EE memory that
 * each side updates by SIF DMA.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class AsyncBlock {
public:
    /** Size of the header, and the granularity of the ring buffer. */
    static constexpr unsigned int kHeaderSize = 0x40;

    /**
     * Turn a buffer into a ring buffer of one empty block.
     *
     * @param size Buffer size, at least two headers.
     * @return Zero, or 2 for a null buffer or a buffer that is too small.
     * @ghidraAddress NTSC-U/C: 0x0000367c
     * @ghidraAddress PAL: 0x000034ec
     */
    int InitRing(unsigned int size);

    /**
     * Measure the empty blocks that start here.
     *
     * @param size Bytes from this block to the end of the ring buffer.
     * @param free Receives the byte count of the empty blocks, headers included.
     * @return Zero, or 2 for a null argument.
     * @ghidraAddress NTSC-U/C: 0x000036c4
     * @ghidraAddress PAL: 0x00003534
     */
    int MeasureFree(unsigned int size, unsigned int *free);

    /**
     * The block that starts a byte offset into a ring buffer.
     *
     * @param ring Ring buffer.
     * @param offset Byte offset of the block.
     * @return The block.
     */
    static AsyncBlock *At(unsigned char *ring, unsigned int offset);

    int mUsed;               /*!< 1 while the block stores a datagram. */
    int mPadding;            /*!< 1 when the block only fills the end of the ring buffer. */
    unsigned int mCapacity;  /*!< Bytes after the header that belong to the block. */
    unsigned int mLength;    /*!< Length of the datagram. */
    sceInetAddress mAddress; /*!< Remote address of the datagram. */
    int mPort;               /*!< Remote port of the datagram. */
    int mFlags;              /*!< Flags of the transfer. */
    int mResult;             /*!< Result of sending the datagram. */
    AsyncBlock *mSelf;       /*!< Address of the block in the memory of the side that filled it. */
    unsigned int mReserved[3]; /* +0x30 */
    int mValid;                /*!< 1 when the datagram is complete. */
    unsigned char mData[];     /*!< The datagram. */
};

#pragma once

/** Number of block buffers. */
const int kNumBlockBuffers = 8;

/** Bytes of one disc sector. */
const int kSectorSize = 2048;

/**
 * Bytes of one block, the unit the block manager reads from the archive.
 *
 * @ghidraAddress 0x10038f0c
 */
extern int kBlockSize;

/**
 * Buffer of one block of the archive.
 *
 * The RTTI includes the class name. The object is 0xc bytes.
 */
class Block {
public:
    /**
     * Take the next free block buffer and mark the block as just used.
     *
     * @ghidraAddress 0x1000f070
     */
    Block();

    /**
     * Mark the block as just used.
     *
     * @ghidraAddress 0x1000f0b0
     */
    void UpdateTimestamp();

    /**
     * Take the index of the next free block buffer.
     *
     * @return The index.
     * @ghidraAddress 0x1000f030
     */
    static int NextBufferIndex();

    /**
     * Report the latest timestamp.
     *
     * @return The timestamp.
     * @ghidraAddress 0x1000f0a0
     */
    static int CurrentTimestamp();

    char *mBuffer;  /*!< The block's data. */
    int mBlockNum;  /*!< The block of the archive in the buffer, or -1. */
    int mTimestamp; /*!< When the block was last used. */
};

/**
 * Number of block buffers taken.
 *
 * @ghidraAddress 0x100c1128
 */
extern int gCurrBuffNum;

/**
 * Latest block timestamp.
 *
 * @ghidraAddress 0x100c112c
 */
extern int gBlockTimestamp;

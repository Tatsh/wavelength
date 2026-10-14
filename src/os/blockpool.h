#pragma once

/**
 * Pool of equal blocks carved from a region, each free block linking to the next.
 *
 * The class is not polymorphic. The name is inferred. The memory tracker takes its AllocInfo
 * records from one.
 */
class BlockPool {
public:
    /**
     * Take over a region as a chain of free blocks.
     *
     * @param nBlockSize The size of each block in bytes.
     * @param nNumBlocks The number of blocks.
     * @param pMemory The region.
     * @ghidraAddress NTSC-U/C: 0x0029daa0
     * @ghidraAddress PAL: 0x002a7768
     */
    void Init(int nBlockSize, int nNumBlocks, void *pMemory);

    /**
     * Hand out the first free block.
     *
     * @return The block, or null when none is free.
     * @ghidraAddress NTSC-U/C: 0x0029db18
     * @ghidraAddress PAL: 0x002a77e0
     */
    void *Alloc();

    /**
     * Return a block to the head of the chain.
     *
     * @param pBlock The block.
     * @ghidraAddress NTSC-U/C: 0x0029db38
     * @ghidraAddress PAL: 0x002a7800
     */
    void Free(void *pBlock);

    /** A free block, which starts with the link to the next one. */
    struct Node {
        Node *mNext; /*!< The next free block, or null. */
    };

    Node *mMemory;  /*!< The region. */
    Node *mFree;    /*!< The first free block, or null. */
    int mBlockSize; /*!< The size of each block in bytes. */
    int mNumBlocks; /*!< The number of blocks. */

private:
    // Fill in the members and link every block of the region.
    // NTSC-U/C: 0x0029dac8, PAL: 0x002a7790
    Node *Setup(int nBlockSize, int nNumBlocks, void *pMemory);
};

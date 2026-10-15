#pragma once

#include "os/prnstream.h"

/**
 * Free block of a Heap, a node of the heap's free list in address order.
 *
 * The class is not polymorphic. The name is inferred.
 */
class FreeBlock {
public:
    /**
     * Absorb the free block that starts where this one ends.
     *
     * @param pNext The following free block.
     * @return Whether pNext starts where this block ends, and was absorbed.
     * @ghidraAddress NTSC-U/C: 0x0029a430
     * @ghidraAddress PAL: 0x002a4050
     */
    bool Merge(FreeBlock *pNext);

    int mSizeWords;   /*!< The size of the block in words, this node included. */
    FreeBlock *mNext; /*!< The next free block, at a higher address, or null. */
};

/** The word before each block Heap::Alloc() hands out. */
struct BlockHeader {
    unsigned int mSizeWords : 24; /*!< The words the block takes, padding and header included. */
    unsigned int mPadWords : 8;   /*!< The cleared words before the header that align the data. */
};

/**
 * Region of memory that hands out word-aligned blocks from a free list, best fit first.
 *
 * The class is not polymorphic. The name is inferred. The heaps are entries of a table that
 * MemRegisterHeap() fills, and PoolMemAlloc() allocates from the heap MemPushHeap() selected.
 */
class Heap {
public:
    /**
     * Take over a region as one free block, starting at the first word whose next word is
     * 16-byte aligned.
     *
     * @param pszName The name of the heap.
     * @param nNum The index of the heap in the table.
     * @param pStart The region.
     * @param nSizeWords The size of the region in words.
     * @param bHandles Non-zero for a heap of handle blocks Compact() may move.
     * @ghidraAddress NTSC-U/C: 0x00299fb8
     * @ghidraAddress PAL: 0x002a3bd8
     */
    void Init(const char *pszName, int nNum, int *pStart, int nSizeWords, int bHandles);

    /**
     * Make a free block of a region and link it between two free blocks.
     *
     * @param pBlock The region.
     * @param nSizeWords The size of the region in words.
     * @param pPrev The free block before, or null for the head of the list.
     * @param pNext The free block after, or null.
     * @ghidraAddress NTSC-U/C: 0x00299f40
     * @ghidraAddress PAL: 0x002a3b60
     */
    void InsertFreeBlock(FreeBlock *pBlock, int nSizeWords, FreeBlock *pPrev, FreeBlock *pNext);

    /**
     * Report the free blocks.
     *
     * @param pnBlocks Receives the number of free blocks.
     * @param pnFreeBytes Receives the free bytes.
     * @param pnBiggest Receives the bytes of the biggest free block.
     * @ghidraAddress NTSC-U/C: 0x00299f60
     * @ghidraAddress PAL: 0x002a3b80
     */
    void GetInfo(int *pnBlocks, int *pnFreeBytes, int *pnBiggest);

    /**
     * Change the size of the heap, growing or shrinking its last free block.
     *
     * The last free block is taken to end the heap.
     *
     * @param nSizeWords The size in words.
     * @ghidraAddress NTSC-U/C: 0x0029a018
     * @ghidraAddress PAL: 0x002a3c38
     */
    void Resize(int nSizeWords);

    /**
     * Hand out a block from the smallest free block that fits.
     *
     * Padding of 9 or more words before the aligned block, and spare words of 9 or more after
     * it, go back to the free list. A failure is reported and then dereferences a null block.
     *
     * @param nSizeWords The words the block needs, its header included.
     * @param pszTag The allocation tag, which the report of a failure leaves out.
     * @param nAlignShift The alignment of the data as a power of two of words.
     * @param pnSizeWords Receives the words the block takes.
     * @return The data of the block, after its header.
     * @ghidraAddress NTSC-U/C: 0x0029a070
     * @ghidraAddress PAL: 0x002a3c90
     */
    void *Alloc(int nSizeWords, const char *pszTag, int nAlignShift, int *pnSizeWords);

    /**
     * Shrink a block of the heap in place, returning its spare words.
     *
     * A block is never grown.
     *
     * @param pData The data of the block.
     * @param nSizeWords The words the data needs.
     * @param pnSizeWords Receives the words the block takes.
     * @return pData, or null when the block is not in the heap.
     * @ghidraAddress NTSC-U/C: 0x0029a2b8
     * @ghidraAddress PAL: 0x002a3ed8
     */
    void *Realloc(void *pData, int nSizeWords, int *pnSizeWords);

    /**
     * Find the free blocks on each side of an address.
     *
     * @param pAddress The address.
     * @param ppPrev Receives the last free block before the address, or null.
     * @param ppNext Receives the first free block at or after the address, or null.
     * @ghidraAddress NTSC-U/C: 0x0029a3f8
     * @ghidraAddress PAL: 0x002a4018
     */
    void FindNeighbors(void *pAddress, FreeBlock **ppPrev, FreeBlock **ppNext);

    /**
     * Return a block to the free list, merging it with its free neighbours.
     *
     * @param pData The data of the block.
     * @return Whether the block was in the heap.
     * @ghidraAddress NTSC-U/C: 0x0029a468
     * @ghidraAddress PAL: 0x002a4088
     */
    bool Free(void *pData);

    /**
     * Write the blocks of the heap in address order.
     *
     * @param stream The stream to write to.
     * @param bSummary Whether each run of allocated blocks is written as one line before each free
     * block, rather than each allocated block on its own line.
     * @ghidraAddress NTSC-U/C: 0x0029a548
     * @ghidraAddress PAL: 0x002a4168
     */
    void Print(PrnStream &stream, bool bSummary);

    /**
     * Report the name of the heap.
     *
     * @return mName.
     * @ghidraAddress NTSC-U/C: 0x0029ae28
     * @ghidraAddress PAL: 0x002a4a48
     */
    const char *Name() const;

    /**
     * Move the handle blocks after the first free block that has some down over it, within a
     * budget, and update the handles.
     *
     * @param nByteBudget The most bytes to move.
     * @param bStrict Whether a block that would cross the budget stays.
     * @ghidraAddress NTSC-U/C: 0x0029aff0
     * @ghidraAddress PAL: 0x002a4c10
     */
    void Compact(int nByteBudget, bool bStrict);

    FreeBlock *mFreeList; /*!< The free blocks in address order. */
    int *mStart;          /*!< The first word of the heap. */
    const char *mName;    /*!< The name of the heap. */
    int mSizeWords;       /*!< The size of the heap in words. */
    int mNum;             /*!< The index of the heap in the table. */
    int mHandles;         /*!< Non-zero for a heap of handle blocks. */
};

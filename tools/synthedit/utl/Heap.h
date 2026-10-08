#pragma once

#include "utl/FreeBlock.h"
#include "utl/PrnStream.h"

/**
 * Word in front of every allocation a heap hands out.
 *
 * Zeroed padding words may precede the header to align the allocation.
 */
struct AllocHeader {
    unsigned int mSizeWords : 24; /*!< Words the allocation occupies with padding and header. */
    unsigned int mPadWords : 8;   /*!< Zeroed words in front of the header. */
};

/**
 * Region of memory carved into allocations with a sorted free list.
 *
 * The object is 0x18 bytes. The memory manager keeps sixteen of them. The name is inferred from the
 * assertions of the memory manager.
 */
class Heap {
public:
    /**
     * Take over a region of memory as one free run.
     *
     * The start is rounded up to 16 bytes, and the size shrinks by the words skipped.
     *
     * @param name The heap's name.
     * @param num The heap's index.
     * @param start The first word of the region.
     * @param sizeWords The size of the region in words.
     * @param flag The third value of the heap's configuration entry.
     * @ghidraAddress 0x10010280
     */
    void Init(const char *name, int num, int *start, int sizeWords, bool flag);

    /**
     * Report the free runs.
     *
     * @param numBlocks Receives the number of free runs.
     * @param freeBytes Receives the bytes in all free runs.
     * @param biggestBlock Receives the bytes in the largest free run.
     * @ghidraAddress 0x10010230
     */
    void FreeBlockStats(int *numBlocks, int *freeBytes, int *biggestBlock);

    /**
     * Grow or shrink the heap by moving its end. The last free run must end at the heap's end.
     *
     * @param sizeWords The new size in words.
     * @ghidraAddress 0x100102f0
     */
    void Resize(int sizeWords);

    /**
     * Allocate from the smallest free run that fits.
     *
     * A failure prints the heap's state through the diagnostic stream.
     *
     * @param sizeWords The words wanted, the header included.
     * @param name The allocation's name. The routine does not read it.
     * @param alignBits The allocation is aligned to 1 << alignBits words.
     * @param allocatedWords Receives the words taken, padding included.
     * @return The allocation, after its header.
     * @ghidraAddress 0x10010380
     */
    void *Alloc(int sizeWords, const char *name, int alignBits, int *allocatedWords);

    /**
     * Return an allocation to the free list, merging it with its neighbours.
     *
     * @param mem The allocation.
     * @return Whether the allocation lies in this heap.
     * @ghidraAddress 0x10010590
     */
    bool Free(void *mem);

    /**
     * Print every run of the heap.
     *
     * @param stream The destination.
     * @param summary When set, runs of allocations print as one line each with their count and
     * free runs print too. When clear, every allocation prints alone and free runs are omitted.
     * @ghidraAddress 0x10010620
     */
    void Print(PrnStream &stream, bool summary);

    /**
     * Report the first word of the heap.
     *
     * @return The first word.
     */
    int *Start() const {
        return mStart;
    }

    /**
     * Report the size of the heap.
     *
     * @return The size in words.
     */
    int SizeWords() const {
        return mSizeWords;
    }

    /**
     * Report the heap's name.
     *
     * @return The name.
     */
    const char *Name() const {
        return mName;
    }

private:
    /**
     * Link a free run between two others.
     *
     * @param iBlock The run.
     * @param sizeWords The length of the run in words.
     * @param iPrevBlock The free run before it, or null when it becomes the first.
     * @param iNextBlock The free run after it, or null.
     * @ghidraAddress 0x100101d0
     */
    void
    InsertFreeBlock(FreeBlock *iBlock, int sizeWords, FreeBlock *iPrevBlock, FreeBlock *iNextBlock);

    /**
     * Find the free runs around an address.
     *
     * @param addr The address.
     * @param prev Receives the last free run below the address, or null.
     * @param next Receives the first free run at or above the address, or null.
     * @ghidraAddress 0x10010530
     */
    void FindNeighbors(int *addr, FreeBlock **prev, FreeBlock **next);

    FreeBlock *mFreeList; /*!< The free run at the lowest address. */
    int *mStart;          /*!< The first word of the heap. */
    const char *mName;    /*!< The heap's name. */
    int mSizeWords;       /*!< The size of the heap in words. */
    int mNum;             /*!< The heap's index. */
    bool mFlag;           // +0x14, stored by Init() and not read by any routine.
};

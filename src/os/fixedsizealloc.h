#pragma once

#include "os/freestore.h"
#include "os/prnstream.h"

/**
 * Allocator of small blocks, with a FreeStore for each multiple of 16 bytes up to 2048.
 *
 * The class is not polymorphic. The name comes from the tag of its stores. PoolAlloc() hands out
 * its blocks.
 */
class FixedSizeAlloc {
public:
    /** The node sizes, one for each 16 bytes. */
    static constexpr int kNumStores = 128;

    /**
     * Construct an allocator with no stores yet.
     *
     * @ghidraAddress NTSC-U/C: 0x0029de20
     * @ghidraAddress PAL: 0x002a7ae8
     */
    FixedSizeAlloc();

    /**
     * Hand out a block from the store of its size, creating the store on first use.
     *
     * @param nSize The size of the block in bytes.
     * @param nUnused Not used.
     * @return The block.
     * @ghidraAddress NTSC-U/C: 0x0029de58
     * @ghidraAddress PAL: 0x002a7b20
     */
    void *Alloc(int nSize, int nUnused);

    /**
     * Return a block to the store of its size.
     *
     * @param pBlock The block.
     * @param nSize The size given to Alloc().
     * @ghidraAddress NTSC-U/C: 0x0029ded8
     * @ghidraAddress PAL: 0x002a7ba0
     */
    void Free(void *pBlock, int nSize);

    /**
     * Write the use of each store and the bytes their free nodes take.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0029df28
     * @ghidraAddress PAL: 0x002a7bf0
     */
    void Report(PrnStream &stream);

    /**
     * Report the store a size of block takes.
     *
     * @param nSize The size in bytes.
     * @return The index of the store.
     * @ghidraAddress NTSC-U/C: 0x0029db50
     * @ghidraAddress PAL: 0x002a7818
     */
    static int StoreIndex(int nSize);

    /**
     * Report the node size of a store.
     *
     * @param nIndex The index of the store.
     * @return The node size in words.
     * @ghidraAddress NTSC-U/C: 0x0029db60
     * @ghidraAddress PAL: 0x002a7828
     */
    static int StoreNodeWords(int nIndex);

    FreeStore *mStores[kNumStores]; /*!< The store of each size, or null before its first use. */
};

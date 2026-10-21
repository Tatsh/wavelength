#pragma once

#include <stddef.h>

#include "os/mem.h"
#include "script/dataarray.h"

/**
 * Free list of nodes of one size, grown a chunk of nodes at a time from the hunks of the pool.
 *
 * The class is not polymorphic. The name is inferred. FixedSizeAlloc keeps one for each node
 * size.
 */
class FreeStore {
public:
    /** A free node, which starts with the link to the next one. */
    struct Node {
        Node *mNext; /*!< The next free node, or null. */
    };

    /**
     * Construct an empty store.
     *
     * @param nNodeWords The size of each node in words.
     * @ghidraAddress NTSC-U/C: 0x0029dc78
     * @ghidraAddress PAL: 0x002a7940
     */
    explicit FreeStore(int nNodeWords);

    /**
     * Allocate a store from the pool heaps.
     *
     * @param nSize The size of the store.
     * @return The block.
     */
    static void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "FixedSizeAlloc", 0);
    }

    /**
     * Hand out a node, growing the store when none is free.
     *
     * @return The node.
     * @ghidraAddress NTSC-U/C: 0x0029dca0
     * @ghidraAddress PAL: 0x002a7968
     */
    void *Alloc();

    /**
     * Return a node.
     *
     * @param pNode The node.
     * @ghidraAddress NTSC-U/C: 0x0029dd00
     * @ghidraAddress PAL: 0x002a79c8
     */
    void Free(void *pNode);

    /**
     * Add a chunk of mNodesPerChunk free nodes.
     *
     * @ghidraAddress NTSC-U/C: 0x0029dd20
     * @ghidraAddress PAL: 0x002a79e8
     */
    void Grow();

    /**
     * Take a chunk of nodes from the hunks of the pool.
     *
     * @param nBytes The size of the chunk.
     * @return The chunk.
     * @ghidraAddress NTSC-U/C: 0x0029ddb0
     * @ghidraAddress PAL: 0x002a7a78
     */
    void *AllocChunk(int nBytes);

    /**
     * Report the use of the store.
     *
     * @param pnNodeBytes Receives the size of each node in bytes.
     * @param pnAllocs Receives the nodes handed out.
     * @param pnMaxAllocs Receives the most nodes handed out at one time.
     * @param pnCapacity Receives the nodes of every chunk.
     * @param pnWasted Receives the bytes of the free nodes.
     * @ghidraAddress NTSC-U/C: 0x0029ddd0
     * @ghidraAddress PAL: 0x002a7a98
     */
    void
    GetStats(int *pnNodeBytes, int *pnAllocs, int *pnMaxAllocs, int *pnCapacity, int *pnWasted);

    /**
     * Read the sizes of the pool's hunks from the `pool` block of the `mem` configuration.
     *
     * The first hunk takes `big_hunk` bytes and every later one `small_hunk` bytes.
     *
     * @param pConfig The `pool` block.
     * @ghidraAddress NTSC-U/C: 0x0029db70
     * @ghidraAddress PAL: 0x002a7838
     */
    static void Configure(DataArray *pConfig);

    /**
     * Carve bytes from the current hunk of the pool, allocating a new hunk when they do not fit.
     *
     * @param nBytes The bytes.
     * @return The bytes.
     * @ghidraAddress NTSC-U/C: 0x0029dbc8
     * @ghidraAddress PAL: 0x002a7890
     */
    static void *AllocHunk(int nBytes);

    /**
     * The bytes AllocHunk() has handed out.
     *
     * @ghidraAddress NTSC-U/C: 0x003b2420
     * @ghidraAddress PAL: 0x004312e0
     */
    static int sHunkBytes;

private:
    int mNodeWords;     /*!< The size of each node in words. */
    int mNumAllocs;     /*!< The nodes handed out. */
    int mMaxAllocs;     /*!< The most nodes handed out at one time. */
    int mNumChunks;     /*!< The chunks Grow() added. */
    Node *mFree;        /*!< The first free node, or null. */
    int mNodesPerChunk; /*!< The nodes of each chunk, 20. */
};

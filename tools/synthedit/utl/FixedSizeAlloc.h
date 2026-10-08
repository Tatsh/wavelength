#pragma once

/** Free node of a FixedSizeAlloc. The rest of the node is unused while it is free. */
struct FixedSizeNode {
    FixedSizeNode *mNext; /*!< The next free node, or null. */
};

/**
 * Free list of equal-sized nodes carved from pool chunks.
 *
 * The object is 0x18 bytes. The name comes from the allocation name its owner passes.
 */
class FixedSizeAlloc {
public:
    /**
     * Create an empty allocator.
     *
     * @param allocSizeWords The node size in words.
     * @ghidraAddress 0x10011880
     */
    explicit FixedSizeAlloc(int allocSizeWords);

    /**
     * Take a node, refilling the free list from a new chunk when it is empty.
     *
     * @return The node.
     * @ghidraAddress 0x100118d0
     */
    void *Alloc();

    /**
     * Return a node to the free list.
     *
     * @param mem The node.
     * @ghidraAddress 0x10011900
     */
    void Free(void *mem);

    /**
     * Report the allocator's use.
     *
     * @param nodeSize Receives the node size in bytes.
     * @param numAllocs Receives the nodes in use.
     * @param maxAllocs Receives the most nodes in use at once.
     * @param capacity Receives the nodes in all chunks.
     * @param wasted Receives the bytes in unused nodes.
     * @ghidraAddress 0x100119a0
     */
    void GetStats(int *nodeSize, int *numAllocs, int *maxAllocs, int *capacity, int *wasted);

private:
    /**
     * Fill the empty free list with the nodes of a new chunk.
     *
     * @ghidraAddress 0x10011920
     */
    void Refill();

    /**
     * Take memory for a chunk.
     *
     * @param bytes The size in bytes.
     * @return The chunk.
     * @ghidraAddress 0x10011990
     */
    void *RawAlloc(int bytes);

    int mAllocSizeWords;      /*!< The node size in words. */
    int mNumAllocs;           /*!< Nodes in use. */
    int mMaxAllocs;           /*!< The most nodes in use at once. */
    int mNumChunks;           /*!< Chunks taken. */
    FixedSizeNode *mFreeList; /*!< The first free node. */
    int mNodesPerChunk;       /*!< Nodes in each chunk. */
};

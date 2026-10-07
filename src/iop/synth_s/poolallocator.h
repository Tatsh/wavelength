#pragma once

/** The link a free element of a PoolAllocator stores in its first word. */
struct PoolLink {
    PoolLink *mNext; /*!< The next free element, or null at the end of the list. */
};

/**
 * A fixed-size allocator over one block of IOP memory.
 *
 * The class has no RTTI, and its name is inferred from its role. Free elements form a singly
 * linked list through their first word. Elements are raw storage, and no constructor runs on
 * them.
 */
class PoolAllocator {
public:
    /**
     * Carve a caller-supplied block into elements.
     *
     * @param elementSize Bytes per element, at least the size of a pointer.
     * @param count Number of elements.
     * @param block The block, at least @p elementSize times @p count bytes.
     * @ghidraAddress NTSC-U/C: 0x000013a0
     * @ghidraAddress PAL: 0x000013a0
     */
    void InitWithBlock(int elementSize, int count, void *block);

    /**
     * Allocate a block and carve it into elements, rounding the element size up to a word.
     *
     * @param elementSize Bytes per element.
     * @param count Number of elements.
     * @ghidraAddress NTSC-U/C: 0x000013c0
     * @ghidraAddress PAL: 0x000013c0
     */
    void Init(int elementSize, int count);

    /**
     * Release the block Init() allocated.
     *
     * @ghidraAddress NTSC-U/C: 0x00001480
     * @ghidraAddress PAL: 0x00001480
     */
    void Release();

    /**
     * Take an element off the free list.
     *
     * @return Raw storage for one element, or null when every element is in use.
     * @ghidraAddress NTSC-U/C: 0x000014a4
     * @ghidraAddress PAL: 0x000014a4
     */
    void *Alloc();

    /**
     * Return an element to the free list.
     *
     * @param element Storage Alloc() returned.
     * @ghidraAddress NTSC-U/C: 0x000014d0
     * @ghidraAddress PAL: 0x000014d0
     */
    void Free(void *element);

private:
    /**
     * Record the block and link its elements into the free list in address order.
     *
     * @param elementSize Bytes per element.
     * @param count Number of elements.
     * @param block The block.
     * @ghidraAddress NTSC-U/C: 0x00001438
     * @ghidraAddress PAL: 0x00001438
     */
    void BuildFreeList(int elementSize, int count, void *block);

    PoolLink *mBlock; /*!< The block, which is also its first element. */
    PoolLink *mFree;  /*!< The first free element, or null when none is free. */
    int mElementSize; /*!< Bytes per element. */
    int mCount;       /*!< Number of elements. */
};

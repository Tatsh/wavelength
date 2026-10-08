#pragma once

#include "utl/FixedSizeAlloc.h"
#include "utl/PrnStream.h"

/** Size classes the chunk allocator serves, 16 bytes apart. */
const int MAX_FIXED_ALLOCS = 128;

/**
 * Small-object allocator with one FixedSizeAlloc for each 16-byte size class up to 2048 bytes.
 *
 * The object is 0x200 bytes. The name comes from the assertions that guard its one instance,
 * `gChunkAlloc`.
 */
class ChunkAllocator {
public:
    /**
     * Create an allocator with no size class in use.
     *
     * @ghidraAddress 0x100119e0
     */
    ChunkAllocator();

    /**
     * Allocate from the size class of a size, creating the class's allocator on first use.
     *
     * @param size The size in bytes.
     * @param unused A value the routine does not read.
     * @return The allocation.
     * @ghidraAddress 0x10011a00
     */
    void *Alloc(int size, int unused);

    /**
     * Free an allocation of a size class.
     *
     * @param mem The allocation.
     * @param size The size it was allocated with.
     * @ghidraAddress 0x10011ae0
     */
    void Free(void *mem, int size);

    /**
     * Print the use of every size class.
     *
     * @param stream The destination.
     * @ghidraAddress 0x10011b60
     */
    void Print(PrnStream &stream);

private:
    /**
     * Find the size class of a size.
     *
     * @param size The size in bytes.
     * @return The class.
     * @ghidraAddress 0x10011ac0
     */
    static int SizeToIndex(int size);

    /**
     * Find the node size of a size class.
     *
     * @param index The class.
     * @return The node size in words.
     * @ghidraAddress 0x10011ad0
     */
    static int IndexToSizeWords(int index);

    FixedSizeAlloc *mAllocs[MAX_FIXED_ALLOCS]; /*!< Each size class's allocator, or null. */
};

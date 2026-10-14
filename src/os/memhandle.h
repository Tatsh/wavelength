#pragma once

#include <stddef.h>

#include "os/mem.h"

class MemHandle;

/** The 16-byte header of a block a MemHandle refers to. The data follows it. */
struct HandleBlock {
    MemHandle *mHandle; /*!< The handle Heap::Compact() updates when it moves the block. */
    int mLockCount;     /*!< The number of MemHandle::Lock() calls not yet undone. */
    int mReserved[2];   // +0x08, padding that puts the data on a 16-byte boundary.
};

/**
 * Movable block of a handle heap, reached through a handle the block points back to.
 *
 * The class is not polymorphic. The name is inferred from the allocation tag "MemHandle".
 */
class MemHandle {
public:
    /**
     * Attach the handle to a block with no locks.
     *
     * @param pBlock The block.
     * @ghidraAddress NTSC-U/C: 0x0029ae60
     * @ghidraAddress PAL: 0x002a4a80
     */
    explicit MemHandle(HandleBlock *pBlock);

    /**
     * Allocate a handle from the pool of 4-byte blocks.
     *
     * @return The block.
     */
    static void *operator new(size_t) {
        return PoolAlloc(sizeof(MemHandle), sizeof(MemHandle), "MemHandle", 0);
    }

    /**
     * Return a handle to the pool of 4-byte blocks.
     *
     * @param pBlock The block.
     */
    static void operator delete(void *pBlock) {
        PoolFree(sizeof(MemHandle), pBlock);
    }

    /**
     * Allocate a block and a handle for it.
     *
     * The block takes the size rounded up to 16 bytes, with its header.
     *
     * @param nBytes The bytes of data.
     * @param pszTag The allocation tag.
     * @return The handle.
     * @ghidraAddress NTSC-U/C: 0x0029aeb0
     * @ghidraAddress PAL: 0x002a4ad0
     */
    static MemHandle *Alloc(int nBytes, const char *pszTag);

    /**
     * Free a handle and its block.
     *
     * A null handle is ignored.
     *
     * @param pHandle The handle.
     * @ghidraAddress NTSC-U/C: 0x0029af40
     * @ghidraAddress PAL: 0x002a4b60
     */
    static void Free(MemHandle *pHandle);

    /**
     * Count a lock and report the data.
     *
     * @return The data of the block.
     * @ghidraAddress NTSC-U/C: 0x0029ae78
     * @ghidraAddress PAL: 0x002a4a98
     */
    void *Lock();

    /**
     * Undo one Lock().
     *
     * @ghidraAddress NTSC-U/C: 0x0029ae98
     * @ghidraAddress PAL: 0x002a4ab8
     */
    void Unlock();

    /**
     * Shrink the block in place through MemRealloc().
     *
     * @param nBytes The bytes of data.
     * @return The block MemRealloc() reports.
     * @ghidraAddress NTSC-U/C: 0x0029af10
     * @ghidraAddress PAL: 0x002a4b30
     */
    void *Resize(int nBytes);

    HandleBlock *mBlock; /*!< The block. */
};

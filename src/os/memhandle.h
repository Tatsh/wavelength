#pragma once

class MemHandle;

/**
 * Header of a block a MemHandle owns.
 *
 * The structure is not polymorphic and emits no RTTI descriptor. The name is inferred. The data
 * starts 16 bytes into the block.
 */
struct MemBlock {
    MemHandle *mOwner;            /*!< The handle that owns the block. */
    int mLocks;                   /*!< The number of Lock() calls not yet undone by Unlock(). */
    unsigned char mReserved08[8]; // +0x08 Never read.
};

/**
 * Handle of a block of the pool heap that the heap may move while the block is unlocked.
 *
 * The class has no virtual and no RTTI. The name is the tag the handles are allocated under.
 */
class MemHandle {
public:
    /**
     * Take ownership of a block and mark it unlocked.
     *
     * @param pBlock The block.
     * @ghidraAddress NTSC-U/C: 0x0029ae60
     * @ghidraAddress PAL: 0x002a4a80
     */
    explicit MemHandle(MemBlock *pBlock);

    /**
     * Pin the block and report where its data is.
     *
     * @return The data, 16 bytes into the block.
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
     * Change the size of the block.
     *
     * @param nSize The data size in bytes.
     * @ghidraAddress NTSC-U/C: 0x0029af10
     * @ghidraAddress PAL: 0x002a4b30
     */
    void Resize(int nSize);

    /**
     * Allocate a block with room for data of a size, rounded up to 16 bytes, and a handle for it.
     *
     * @param nSize The data size in bytes.
     * @param pszTag The tag the block is billed to.
     * @return The handle.
     * @ghidraAddress NTSC-U/C: 0x0029aeb0
     * @ghidraAddress PAL: 0x002a4ad0
     */
    static MemHandle *Alloc(int nSize, const char *pszTag);

    /**
     * Release a handle and its block.
     *
     * A null handle is ignored.
     *
     * @param pHandle The handle, or null.
     * @ghidraAddress NTSC-U/C: 0x0029af40
     * @ghidraAddress PAL: 0x002a4b60
     */
    static void Free(MemHandle *pHandle);

    MemBlock *mBlock; /*!< The block. */
};

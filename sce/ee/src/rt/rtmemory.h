#ifndef RTMEMORY_H
#define RTMEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Heap blocks of the SCE-RT library. The blocks come from the allocator rtSetAllocHooks() installs,
 * or from the C heap when none is installed. Every routine reports 0 for success.
 */

/** A block with a pointer into it rounded up to an alignment. */
typedef struct {
    void *pBlock;   /*!< The block, as the allocator returned it. */
    void *pAligned; /*!< The first aligned address of the block. */
} RtAlignedBlock;

/**
 * Allocate a block filled with zeroes.
 *
 * @param ppBlock Receives the block, or null when the allocation failed.
 * @param nSize The size in bytes.
 * @return 0, or 1 when the allocation failed or ppBlock is null.
 * @ghidraAddress NTSC-U/C: 0x002aa464
 * @ghidraAddress PAL: 0x002b43d4
 */
int RtAllocZeroedBlock(void **ppBlock, int nSize);

/**
 * Release a block and clear the pointer to it. A null block is not released.
 *
 * @param ppBlock The pointer to the block.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002aa4c8
 * @ghidraAddress PAL: 0x002b4438
 */
int RtFreeBlock(void **ppBlock);

/**
 * Allocate a block with an aligned address in it, and fill the aligned part with zeroes.
 *
 * @param pBlock Receives the block and its aligned address.
 * @param nSize The size in bytes the aligned address must have after it.
 * @param nAlign The alignment in bytes. A value below 2 asks for none.
 * @return 0, or 1 when the allocation failed or an argument is null or zero.
 * @ghidraAddress NTSC-U/C: 0x002aa5b8
 * @ghidraAddress PAL: 0x002b4528
 */
int RtAllocAlignedZeroedBlock(RtAlignedBlock *pBlock, unsigned int nSize, unsigned short nAlign);

/**
 * Release a block that RtAllocAlignedBlock() allocated, and clear both pointers.
 *
 * @param pBlock The block.
 * @return 0, 1 when pBlock is null, or the result of RtFreeBlock().
 * @ghidraAddress NTSC-U/C: 0x002aa62c
 * @ghidraAddress PAL: 0x002b459c
 */
int RtFreeAlignedBlock(RtAlignedBlock *pBlock);

#ifdef __cplusplus
}
#endif

#endif

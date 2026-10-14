#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
#include <new>

#include "os/prnstream.h"
#endif

#ifdef __cplusplus
/**
 * Allocate a block for an array from the pool heaps, the game's global `operator new[]`.
 *
 * The allocation is billed to the tag `new[]`.
 *
 * @param nSize The block size in bytes.
 * @return The block.
 * @ghidraAddress NTSC-U/C: 0x0029b1f0
 * @ghidraAddress PAL: 0x002a4e10
 */
void *operator new[](size_t nSize);

/**
 * Allocate a block for a single object from the pool heaps, the game's global `operator new`.
 *
 * The allocation is billed to the tag `new`.
 *
 * @param nSize The block size in bytes.
 * @return The block.
 * @ghidraAddress NTSC-U/C: 0x0029b1a8
 * @ghidraAddress PAL: 0x002a4dc8
 */
void *operator new(size_t nSize);

/**
 * Allocate an over-aligned object from the pool heaps.
 *
 * The original compiler had no aligned `operator new`, so every allocation used the pool heaps.
 * Without this form, an object with an `alignas` member would be allocated by the C runtime, whose
 * heap the pool heaps leave empty.
 *
 * @param nSize The block size in bytes.
 * @param nAlign The alignment in bytes.
 * @return The block.
 */
void *operator new(size_t nSize, std::align_val_t nAlign);

/**
 * Allocate an over-aligned array from the pool heaps.
 *
 * @param nSize The block size in bytes.
 * @param nAlign The alignment in bytes.
 * @return The block.
 */
void *operator new[](size_t nSize, std::align_val_t nAlign);
#endif

/**
 * Allocate a block from the small-block pool and bill it to a tag.
 *
 * The pool is created on first use.
 *
 * @param nSize The block size in bytes, which selects the node size of the pool.
 * @param nBlockSize Not used.
 * @param pszTag The tag to bill the allocation to.
 * @param nUnused Not used.
 * @return The block.
 * @ghidraAddress NTSC-U/C: 0x0029e010
 * @ghidraAddress PAL: 0x002a7cd8
 */
void *PoolAlloc(int nSize, int nBlockSize, const char *pszTag, int nUnused);

/**
 * Return a block to the small-block pool.
 *
 * @param nSize The block size given to PoolAlloc().
 * @param pBlock The block.
 * @ghidraAddress NTSC-U/C: 0x0029e0a8
 * @ghidraAddress PAL: 0x002a7d70
 */
void PoolFree(int nSize, void *pBlock);

/**
 * Write the use of the small-block pool.
 *
 * The name is inferred.
 *
 * @param stream The stream to write to.
 * @ghidraAddress NTSC-U/C: 0x0029e100
 * @ghidraAddress PAL: 0x002a7dc8
 */
void PoolReport(PrnStream &stream);

/**
 * Report the heap that PoolMemAlloc() allocates from, the top of the heap stack.
 *
 * The name is inferred.
 *
 * @return The index of the heap.
 * @ghidraAddress NTSC-U/C: 0x00299de0
 * @ghidraAddress PAL: 0x002a39e0
 */
int MemGetHeap();

/**
 * Report the free blocks of a heap.
 *
 * The name is inferred.
 *
 * @param nHeap The index of the heap.
 * @param pnBlocks Receives the number of free blocks.
 * @param pnFreeBytes Receives the free bytes.
 * @param pnBiggest Receives the bytes of the biggest free block.
 * @ghidraAddress NTSC-U/C: 0x00299e08
 * @ghidraAddress PAL: 0x002a3a08
 */
void MemHeapInfo(int nHeap, int *pnBlocks, int *pnFreeBytes, int *pnBiggest);

/**
 * Add a heap to the table.
 *
 * The name is inferred.
 *
 * @param pszName The name of the heap.
 * @param pStart The region of the heap.
 * @param nSizeWords The size of the region in words.
 * @param bHandles Non-zero for a heap of handle blocks.
 * @return The index of the heap.
 * @ghidraAddress NTSC-U/C: 0x00299eb0
 * @ghidraAddress PAL: 0x002a3ad0
 */
int MemRegisterHeap(const char *pszName, int *pStart, int nSizeWords, int bHandles);

/**
 * Change the size of a heap.
 *
 * The name is inferred.
 *
 * @param nHeap The index of the heap.
 * @param nSizeWords The size in words.
 * @ghidraAddress NTSC-U/C: 0x00299f10
 * @ghidraAddress PAL: 0x002a3b30
 */
void MemSetHeapSize(int nHeap, int nSizeWords);

/**
 * Make a heap the one PoolMemAlloc() allocates from, until MemPopHeap().
 *
 * The depth of the stack is not checked. The name is inferred.
 *
 * @param nHeap The index of the heap.
 * @ghidraAddress NTSC-U/C: 0x0029a728
 * @ghidraAddress PAL: 0x002a4348
 */
void MemPushHeap(int nHeap);

/**
 * Undo the latest MemPushHeap().
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029a750
 * @ghidraAddress PAL: 0x002a4370
 */
void MemPopHeap();

/**
 * Take the memory from the end of the C library's heap to 0x1f70000 as the heap `main`, and start
 * the memory tracker when it is available.
 *
 * PoolMemAlloc() calls this before the first allocation. The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029a768
 * @ghidraAddress PAL: 0x002a4388
 */
void MemInit();

/**
 * Shrink a block in place.
 *
 * A size of 0 frees the block. A block outside every heap is returned as it is.
 *
 * @param pBlock The block, or null.
 * @param nBytes The size in bytes.
 * @return The block, or null after a free.
 * @ghidraAddress NTSC-U/C: 0x0029aa60
 * @ghidraAddress PAL: 0x002a4680
 */
void *MemRealloc(void *pBlock, int nBytes);

/**
 * Report the number of heaps.
 *
 * The name is inferred.
 *
 * @return The number of heaps.
 * @ghidraAddress NTSC-U/C: 0x0029ae18
 * @ghidraAddress PAL: 0x002a4a38
 */
int MemNumHeaps();

/**
 * Write each block of a heap.
 *
 * The name is inferred.
 *
 * @param nHeap The index of the heap.
 * @param stream The stream to write to.
 * @ghidraAddress NTSC-U/C: 0x0029ae30
 * @ghidraAddress PAL: 0x002a4a50
 */
void MemPrintHeap(int nHeap, PrnStream &stream);

/**
 * Non-zero while the memory tracker records allocations.
 *
 * @ghidraAddress NTSC-U/C: 0x003b23b4
 */
extern int g_nMemTrackingEnabled;

/**
 * Non-zero when the memory tracker bills STL allocations to their container, from `track_stl`.
 *
 * @ghidraAddress NTSC-U/C: 0x003b23b8
 */
extern int g_nTrackStl;

/**
 * Allocate a block of up to 128 bytes from the pool of fixed-size blocks, and a larger block from
 * the pool heaps.
 *
 * The name is inferred.
 *
 * @param nSize The block size in bytes.
 * @param pszTag The tag to bill the allocation to.
 * @return The block, or null for a size of 0.
 * @ghidraAddress NTSC-U/C: 0x0029e128
 * @ghidraAddress PAL: 0x002a7df0
 */
void *MemOrPoolAlloc(int nSize, const char *pszTag);

/**
 * Return a block MemOrPoolAlloc() handed out.
 *
 * A null block is ignored. The name is inferred.
 *
 * @param nSize The size MemOrPoolAlloc() was given.
 * @param pBlock The block.
 * @ghidraAddress NTSC-U/C: 0x0029e170
 * @ghidraAddress PAL: 0x002a7e38
 */
void MemOrPoolFree(int nSize, void *pBlock);

#ifdef __cplusplus
/**
 * Release an array block through PoolMemFree(), the game's global `operator delete[]`.
 *
 * @param pBlock The block to release.
 * @ghidraAddress NTSC-U/C: 0x0029b218
 * @ghidraAddress PAL: 0x002a4e38
 */
void operator delete[](void *pBlock) noexcept;

/**
 * Release a single object through PoolMemFree(), the game's global `operator delete`.
 *
 * @param pBlock The block to release.
 * @ghidraAddress NTSC-U/C: 0x0029b1d0
 * @ghidraAddress PAL: 0x002a4df0
 */
void operator delete(void *pBlock) noexcept;

/**
 * Release an over-aligned object through PoolMemFree().
 *
 * @param pBlock The block to release.
 * @param nAlign The alignment the block was allocated with.
 */
void operator delete(void *pBlock, std::align_val_t nAlign) noexcept;

/**
 * Release an over-aligned array through PoolMemFree().
 *
 * @param pBlock The block to release.
 * @param nAlign The alignment the block was allocated with.
 */
void operator delete[](void *pBlock, std::align_val_t nAlign) noexcept;
#endif

/**
 * Look up a heap by its configured name.
 *
 * The heap table has one 0x18-byte entry per heap, and the lookup compares the name of each entry
 * in table order.
 *
 * @param pszName The heap name, for example "rnd".
 * @return The table index of the heap, or -1 when no heap has that name.
 * @ghidraAddress NTSC-U/C: 0x0029ad90
 * @ghidraAddress PAL: 0x002a49b0
 */
int MemFindHeap(const char *pszName);

/**
 * Make a heap the one allocations come from until the matching MemPopHeap().
 *
 * The name is inferred.
 *
 * @param nHeap The table index MemFindHeap() reported.
 * @ghidraAddress NTSC-U/C: 0x0029a728
 * @ghidraAddress PAL: 0x002a4348
 */
void MemPushHeap(int nHeap);

/**
 * Return to the heap in use before the last MemPushHeap().
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029a750
 * @ghidraAddress PAL: 0x002a4370
 */
void MemPopHeap();

/**
 * Report how many heaps the heap table has.
 *
 * The name is inferred.
 *
 * @return The heap count.
 * @ghidraAddress NTSC-U/C: 0x0029ae18
 * @ghidraAddress PAL: 0x002a4a38
 */
int MemNumHeaps();

/**
 * Move live blocks of one heap together to merge its free space.
 *
 * A negative heap index is ignored. With bStrictBudget set, compaction stops before the first block
 * that would take the bytes moved past the budget. Otherwise compaction stops only after the bytes
 * moved exceed the budget, and the last block moved may cross it.
 *
 * @param nHeap The table index MemFindHeap() reported.
 * @param nByteBudget The most bytes to move in this call.
 * @param bStrictBudget Whether the budget is a hard limit.
 * @ghidraAddress NTSC-U/C: 0x0029af78
 * @ghidraAddress PAL: 0x002a4b98
 */
void MemCompact(int nHeap, int nByteBudget, bool bStrictBudget);

/**
 * Allocate a block from the pool heaps and bill it to a tag.
 *
 * Every class-specific `operator new` forwards here with its class name as the tag, and the global
 * `operator new` passes the tag `new`.
 *
 * @param nSize The block size in bytes.
 * @param pszTag The tag to bill the allocation to.
 * @param nAlign The alignment in bytes, a power of two of at least four. Zero selects 16 bytes.
 * @return The block.
 * @ghidraAddress NTSC-U/C: 0x0029ab68
 * @ghidraAddress PAL: 0x002a4788
 */
void *PoolMemAlloc(int nSize, const char *pszTag, int nAlign);

/**
 * Return a block PoolMemAlloc() handed out to the pool heap that owns it.
 *
 * A null block is ignored.
 *
 * @param pBlock The block.
 * @ghidraAddress NTSC-U/C: 0x0029acd0
 * @ghidraAddress PAL: 0x002a48f0
 */
void PoolMemFree(void *pBlock);

/**
 * Set the heaps up from the system configuration.
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029a820
 * @ghidraAddress PAL: 0x002a4440
 */
void MemConfigureHeaps();

/**
 * Shut the heaps down.
 *
 * The body does nothing. The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029aa58
 * @ghidraAddress PAL: 0x002a4678
 */
void MemTerminate();

/**
 * Record the state of every tracked heap, once per frame.
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029c6c8
 * @ghidraAddress PAL: 0x002a62f0
 */
void MemPoll();

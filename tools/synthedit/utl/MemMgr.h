#pragma once

#include "utl/Heap.h"
#include "utl/PrnStream.h"

/** Number of heaps the memory manager can hold. */
const int MAX_HEAPS = 16;

/**
 * The heaps.
 *
 * @ghidraAddress 0x100c1130
 */
extern Heap gHeaps[MAX_HEAPS];

/**
 * Number of heaps in use.
 *
 * @ghidraAddress 0x100c13b8
 */
extern int gNumHeaps;

/**
 * Whether allocations are reported to the memory tracker.
 *
 * @ghidraAddress 0x100c13c5
 */
extern bool gMemTracking;

/**
 * Whether STL allocations are named after their element type for the memory tracker.
 *
 * @ghidraAddress 0x100c13c6
 */
extern bool gTrackStl;

/**
 * Report the heap new allocations come from.
 *
 * @return The heap on top of the heap stack.
 * @ghidraAddress 0x100100f0
 */
int MemCurrentHeap();

/**
 * Report the free runs of a heap.
 *
 * @param heapNum The heap.
 * @param numBlocks Receives the number of free runs.
 * @param freeBytes Receives the bytes in all free runs.
 * @param biggestBlock Receives the bytes in the largest free run.
 * @ghidraAddress 0x10010100
 */
void MemFreeBlockStats(int heapNum, int *numBlocks, int *freeBytes, int *biggestBlock);

/**
 * Take the memory the heaps are carved from from the C runtime.
 *
 * @param begin Receives the first word.
 * @param end Receives the word past the end.
 * @ghidraAddress 0x10010150
 */
void MemGetSystemMemory(int **begin, int **end);

/**
 * Add a heap.
 *
 * @param name The heap's name.
 * @param start The first word of the heap.
 * @param sizeWords The size of the heap in words.
 * @param flag The third value of the heap's configuration entry.
 * @return The new heap's index.
 * @ghidraAddress 0x10010170
 */
int MemAddHeap(const char *name, int *start, int sizeWords, bool flag);

/**
 * Move the end of a heap.
 *
 * @param heapNum The heap.
 * @param sizeWords The new size in words.
 * @ghidraAddress 0x100101b0
 */
void MemResizeHeap(int heapNum, int sizeWords);

/**
 * Make a heap the source of new allocations until MemPopHeap().
 *
 * @param iHeap The heap.
 * @ghidraAddress 0x10010770
 */
void MemPushHeap(int iHeap);

/**
 * Return to the heap in use before the last MemPushHeap().
 *
 * @ghidraAddress 0x100107e0
 */
void MemPopHeap();

/**
 * Set up the main heap over all system memory.
 *
 * MemAlloc() calls the routine when no heap exists yet.
 *
 * @ghidraAddress 0x100107f0
 */
void MemPreInit();

/**
 * Split system memory into the heaps of the `heaps_pc` configuration and set up the pools.
 *
 * Only the first call has an effect.
 *
 * @ghidraAddress 0x10010870
 */
void MemInit();

/**
 * Allocate from the current heap.
 *
 * @param size The size in bytes.
 * @param name The allocation's name for the memory tracker.
 * @param align The alignment in bytes, or zero for the default of 16.
 * @return The allocation.
 * @ghidraAddress 0x10010a90
 */
void *MemAlloc(int size, const char *name, int align);

/**
 * Free an allocation of any heap. A pointer outside every heap fails.
 *
 * @param mem The allocation, or null.
 * @ghidraAddress 0x10010b80
 */
void MemFree(void *mem);

/**
 * Find a heap by name.
 *
 * @param name The name.
 * @return The heap's index, or -1.
 * @ghidraAddress 0x10010c40
 */
int MemFindHeap(const char *name);

/**
 * Print every allocation of a heap.
 *
 * @param heapNum The heap.
 * @param stream The destination.
 * @ghidraAddress 0x10010ca0
 */
void MemPrintHeap(int heapNum, PrnStream &stream);

#pragma once

#include "os/blockpool.h"
#include "os/heaptracker.h"
#include "os/prnstream.h"
#include "script/dataarray.h"

/** The heaps the memory tracker can follow. */
constexpr int kMemTrackMaxHeaps = 16;

/**
 * The tracker of each heap, or null.
 *
 * @ghidraAddress NTSC-U/C: 0x003b23c8
 * @ghidraAddress PAL: 0x00431288
 */
extern HeapTracker *g_apMemTrackHeaps[kMemTrackMaxHeaps];

/**
 * The pool of the AllocInfo records of every tracker.
 *
 * @ghidraAddress NTSC-U/C: 0x005155c0
 */
extern BlockPool g_AllocInfoPool;

/**
 * Prepare the pool of AllocInfo records, when the development memory is present.
 *
 * The name is inferred.
 *
 * @return Whether memory tracking is available.
 * @ghidraAddress NTSC-U/C: 0x0029c268
 * @ghidraAddress PAL: 0x002a5e88
 */
bool MemTrackInit();

/**
 * Track a heap, or move the end of a heap already tracked.
 *
 * The name is inferred.
 *
 * @param nHeap The index of the heap.
 * @param pStart The first word of the heap.
 * @param pEnd The end of the heap.
 * @ghidraAddress NTSC-U/C: 0x0029c2c8
 * @ghidraAddress PAL: 0x002a5ee8
 */
void MemTrackAddHeap(int nHeap, int *pStart, int *pEnd);

/**
 * Register the script command `heap_report`, add the heap `debug` for the reports, and create the
 * statistics tables of each tracker.
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029c4e0
 * @ghidraAddress PAL: 0x002a6108
 */
void MemTrackConfigure();

/**
 * Record an allocation from the heap MemPushHeap() selected.
 *
 * The name is inferred.
 *
 * @param nRequested The bytes requested.
 * @param pszTag The allocation tag.
 * @param nActual The bytes handed out.
 * @param pData The allocation.
 * @ghidraAddress NTSC-U/C: 0x0029c580
 * @ghidraAddress PAL: 0x002a61a8
 */
void MemTrackAlloc(int nRequested, const char *pszTag, int nActual, void *pData);

/**
 * Remove the record of an allocation of a heap.
 *
 * The name is inferred.
 *
 * @param nHeap The index of the heap.
 * @param pData The allocation.
 * @ghidraAddress NTSC-U/C: 0x0029c5f8
 * @ghidraAddress PAL: 0x002a6220
 */
void MemTrackFree(int nHeap, void *pData);

/**
 * Move the record of an allocation of a heap that changed size or address.
 *
 * The name is inferred.
 *
 * @param nHeap The index of the heap.
 * @param pOld The allocation before the change.
 * @param nRequested The bytes requested, or -1 to retain them.
 * @param nActual The bytes handed out, or -1 to retain them.
 * @param pNew The allocation after the change.
 * @ghidraAddress NTSC-U/C: 0x0029c630
 * @ghidraAddress PAL: 0x002a6258
 */
void MemTrackRealloc(int nHeap, void *pOld, int nRequested, int nActual, void *pNew);

/**
 * Mark a block of the small-block pool, which lies in heap 0.
 *
 * The name is inferred.
 *
 * @param nSize The size of the block.
 * @param pszTag The tag of the block.
 * @param pData The block.
 * @ghidraAddress NTSC-U/C: 0x0029c668
 * @ghidraAddress PAL: 0x002a6290
 */
void MemTrackPoolAlloc(int nSize, const char *pszTag, void *pData);

/**
 * Remove the marks of a block of the small-block pool.
 *
 * The name is inferred.
 *
 * @param pData The block.
 * @ghidraAddress NTSC-U/C: 0x0029c6a0
 * @ghidraAddress PAL: 0x002a62c8
 */
void MemTrackPoolFree(void *pData);

/**
 * Take 16-byte aligned memory from the development memory above 32 MB, which nothing returns.
 *
 * The name is inferred.
 *
 * @param nBytes The bytes.
 * @return The memory, or null without the development memory.
 * @ghidraAddress NTSC-U/C: 0x0029c738
 * @ghidraAddress PAL: 0x002a6360
 */
void *MemTrackBumpAlloc(int nBytes);

/**
 * Write the time, the report of a heap, and its changes since the previous report.
 *
 * A development image writes nothing. The name is inferred.
 *
 * @param nHeap The index of the heap.
 * @param nMinSize The fewest bytes handed out for which a tag is listed.
 * @param stream The stream to write to.
 * @ghidraAddress NTSC-U/C: 0x0029c028
 * @ghidraAddress PAL: 0x002a5c48
 */
void MemTrackReport(int nHeap, int nMinSize, PrnStream &stream);

/**
 * Write the allocations of a heap counted by record.
 *
 * The name is inferred.
 *
 * @param nHeap The index of the heap.
 * @param stream The stream to write to.
 * @ghidraAddress NTSC-U/C: 0x0029c0d8
 * @ghidraAddress PAL: 0x002a5cf8
 */
void MemTrackReportByRecord(int nHeap, PrnStream &stream);

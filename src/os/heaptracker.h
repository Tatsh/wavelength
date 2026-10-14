#pragma once

#include <stddef.h>

#include "os/allocinfo.h"
#include "os/blockstattable.h"
#include "os/prnstream.h"

/**
 * Tracker of the allocations of one heap, with one AllocInfo slot for each word of the heap.
 *
 * The class is not polymorphic. The name is inferred. MemTrackAddHeap() creates one for each heap
 * while memory tracking is available.
 */
class HeapTracker {
public:
    /** The frames of allocation counts Poll() retains. */
    static constexpr int kHistorySize = 20;

    /** Allocations and frees of one frame. */
    struct FrameCounts {
        int mAllocs; /*!< The allocations of the frame. */
        int mFrees;  /*!< The frees of the frame. */
    };

    /**
     * Track a heap, with a cleared slot for each word.
     *
     * @param nHeap The index of the heap.
     * @param pStart The first word of the heap.
     * @param pEnd The end of the heap.
     * @ghidraAddress NTSC-U/C: 0x0029b240
     * @ghidraAddress PAL: 0x002a4e60
     */
    HeapTracker(int nHeap, int *pStart, int *pEnd);

    /**
     * Take a tracker from the memory tracker's own region.
     *
     * @param nSize The size of the tracker.
     * @return The tracker.
     */
    static void *operator new(size_t nSize);

    /**
     * Ignore the release of a tracker. The memory tracker's region never takes memory back.
     */
    static void operator delete(void *) noexcept {
    }

    /**
     * Move the end of the heap after the heap was resized.
     *
     * mStart stays as it was.
     *
     * @param pStart The first word of the heap.
     * @param pEnd The end of the heap.
     * @ghidraAddress NTSC-U/C: 0x0029b2e8
     * @ghidraAddress PAL: 0x002a4f08
     */
    void SetRange(int *pStart, int *pEnd);

    /**
     * Create the two pairs of statistics tables the reports fill.
     *
     * @ghidraAddress NTSC-U/C: 0x0029b300
     * @ghidraAddress PAL: 0x002a4f20
     */
    void CreateTables();

    /**
     * Record an allocation.
     *
     * @param nRequested The bytes requested.
     * @param pszTag The allocation tag.
     * @param nActual The bytes handed out.
     * @param pData The allocation.
     * @ghidraAddress NTSC-U/C: 0x0029b368
     * @ghidraAddress PAL: 0x002a4f88
     */
    void Alloc(int nRequested, const char *pszTag, int nActual, void *pData);

    /**
     * Move the record of an allocation that changed size or address.
     *
     * @param pOld The allocation before the change.
     * @param nRequested The bytes requested, or -1 to retain them.
     * @param nActual The bytes handed out, or -1 to retain them.
     * @param pNew The allocation after the change.
     * @ghidraAddress NTSC-U/C: 0x0029b460
     * @ghidraAddress PAL: 0x002a5080
     */
    void Realloc(void *pOld, int nRequested, int nActual, void *pNew);

    /**
     * Mark the record at an address as a pool block, creating the record when there is none.
     *
     * @param nSize The size of the pool block.
     * @param pszTag The tag of the pool block.
     * @param pData The pool block.
     * @ghidraAddress NTSC-U/C: 0x0029b500
     * @ghidraAddress PAL: 0x002a5120
     */
    void PoolAlloc(int nSize, const char *pszTag, void *pData);

    /**
     * Remove the record of an allocation, or only its pool block marks.
     *
     * @param pData The allocation.
     * @param bPool Whether a pool block is freed. Its record then goes only when it has no tag.
     * @ghidraAddress NTSC-U/C: 0x0029b598
     * @ghidraAddress PAL: 0x002a51b8
     */
    void Free(void *pData, bool bPool);

    /**
     * Store the counts of the frame in the history and start new ones.
     *
     * @ghidraAddress NTSC-U/C: 0x0029b640
     * @ghidraAddress PAL: 0x002a5260
     */
    void Poll();

    /**
     * Report the bytes of the heap not handed out.
     *
     * @return The free bytes.
     * @ghidraAddress NTSC-U/C: 0x0029b698
     * @ghidraAddress PAL: 0x002a52b8
     */
    int FreeBytes();

    /**
     * Write the number of allocations with each distinct record, using the heap `"debug"` for the
     * report's own memory when it exists.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0029b6b8
     * @ghidraAddress PAL: 0x002a52d8
     */
    void ReportByRecord(PrnStream &stream);

    /**
     * Write the totals of the heap and the totals of each tag.
     *
     * @param stream The stream to write to.
     * @param nMinSize The fewest bytes handed out for which a tag is listed.
     * @ghidraAddress NTSC-U/C: 0x0029b950
     * @ghidraAddress PAL: 0x002a5570
     */
    void Report(PrnStream &stream, int nMinSize);

    /**
     * Write the changes of each tag since the previous report, and switch tables.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0029bd30
     * @ghidraAddress PAL: 0x002a5950
     */
    void ReportDiff(PrnStream &stream);

    int mHeap;                          /*!< The index of the heap. */
    int *mStart;                        /*!< The first word of the heap. */
    int *mEnd;                          /*!< The end of the heap. */
    int mSizeWords;                     /*!< The size of the heap in words. */
    AllocInfo **mInfos;                 /*!< One record slot for each word, or null. */
    int mNumAllocs;                     /*!< The allocations. */
    int mBytesActual;                   /*!< The bytes handed out. */
    int mPeakNumAllocs;                 /*!< The most allocations at one time. */
    int mPeakBytes;                     /*!< The most bytes handed out at one time. */
    int mAllocsThisFrame;               /*!< The allocations since the last Poll(). */
    int mFreesThisFrame;                /*!< The frees since the last Poll(). */
    int mHistoryIndex;                  /*!< The number of Poll() calls. */
    BlockStatTable *mStats[2];          /*!< The tag totals of the two latest reports. */
    BlockStatTable *mPoolStats[2];      /*!< The pool tag totals of the two latest reports. */
    int mCurrentStats;                  /*!< The tables the next report fills, 0 or 1. */
    FrameCounts mHistory[kHistorySize]; /*!< The counts of recent frames. */
};

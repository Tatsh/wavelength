#pragma once

#include "utl/AllocInfo.h"
#include "utl/Data.h"
#include "utl/MemStats.h"
#include "utl/PrnStream.h"

/**
 * Allocation tracker for one heap.
 *
 * A mirror array holds an AllocInfo pointer for every word of the heap where an allocation starts.
 * Two pairs of BlockStatTable alternate between reports. Each report prints the change since the
 * previous one. The object is 0xe4 bytes. The name comes from the source file.
 */
class MemTrack {
public:
    /**
     * Start tracking a heap.
     *
     * @param heap The heap's index.
     * @param begin The heap's first word.
     * @param end The word past the heap.
     * @ghidraAddress 0x100177f0
     */
    MemTrack(int heap, int *begin, int *end);

    /**
     * Follow a heap that grew or shrank. The start must not move.
     *
     * @param heapBegin The heap's first word.
     * @param heapEnd The word past the heap.
     * @ghidraAddress 0x10017880
     */
    void SetHeapRange(int *heapBegin, int *heapEnd);

    /**
     * Create the statistics tables.
     *
     * @ghidraAddress 0x10017920
     */
    void CreateStatTables();

    /**
     * Record a heap allocation.
     *
     * @param size The bytes requested.
     * @param type The allocation's name.
     * @param actual The bytes allocated.
     * @param ptr The allocation.
     * @ghidraAddress 0x100179c0
     */
    void Alloc(int size, const char *type, int actual, void *ptr);

    /**
     * Record a pool node.
     *
     * @param size The node size in bytes.
     * @param type The node's name.
     * @param ptr The node.
     * @ghidraAddress 0x10017b00
     */
    void PoolAlloc(int size, const char *type, void *ptr);

    /**
     * Forget an allocation or a pool node.
     *
     * @param ptr The allocation or node.
     * @param pooled Whether a pool node is freed.
     * @ghidraAddress 0x10017bd0
     */
    void Free(void *ptr, bool pooled);

    /**
     * Report the bytes of the heap not allocated.
     *
     * @return The free bytes.
     * @ghidraAddress 0x10017d20
     */
    int BytesFree() const;

    /**
     * Print how many allocations share each name and size.
     *
     * @param stream The destination.
     * @ghidraAddress 0x10017d30
     */
    void ReportAllocsByName(PrnStream &stream);

    /**
     * Print the heap's totals and its allocations by name.
     *
     * @param stream The destination.
     * @param minSize The smallest total, in bytes allocated, that prints.
     * @ghidraAddress 0x10017fe0
     */
    void Report(PrnStream &stream, int minSize);

    /**
     * Print the change of every total since the previous report, then swap the tables.
     *
     * @param stream The destination.
     * @ghidraAddress 0x100182e0
     */
    void DiffReport(PrnStream &stream);

    /**
     * Report whether the mirror array was allocated.
     *
     * @return Whether tracking works.
     */
    bool InitOK() const {
        return mMemMirror != NULL;
    }

private:
    /**
     * Find the mirror slot of an allocation.
     *
     * @param ptr The allocation.
     * @return The slot.
     */
    AllocInfo **MirrorSlot(void *ptr) const {
        return mMemMirror + (static_cast<int *>(ptr) - mHeapBegin);
    }

    int mHeap;                       /*!< The heap's index. */
    int *mHeapBegin;                 /*!< The heap's first word. */
    int *mHeapEnd;                   /*!< The word past the heap. */
    int mMemMirrorSize;              /*!< Words in the heap and slots in the mirror. */
    AllocInfo **mMemMirror;          /*!< One slot for each word of the heap. */
    int mCurNumAllocs;               /*!< Allocations alive. */
    int mCurByteAllocs;              /*!< Bytes allocated to them. */
    int mMaxNumAllocs;               /*!< The most allocations alive at once. */
    int mMaxByteAllocs;              /*!< The most bytes allocated at once. */
    int mTotalAllocs;                /*!< Allocations made. */
    int mTotalFrees;                 /*!< Allocations freed. */
    int mReserved2c;                 // +0x2c, cleared by the constructor and not read.
    BlockStatTable *mStats[2];       /*!< Heap totals of the current and previous report. */
    BlockStatTable *mPoolStats[2];   /*!< Pool totals of the current and previous report. */
    int mCurStat;                    /*!< Which of the two tables the next report fills. */
    unsigned char mReserved44[0xa0]; // +0x44, allocated with the object and not used.
};

/**
 * Report whether allocation tracking is built in. This build returns false.
 *
 * @return Whether tracking is available.
 * @ghidraAddress 0x10018670
 */
bool MemTrackEnabled();

/**
 * Start tracking a heap, or follow its new size.
 *
 * @param heap The heap's index.
 * @param begin The heap's first word.
 * @param end The word past the heap.
 * @ghidraAddress 0x10018680
 */
void MemTrackInit(int heap, int *begin, int *end);

/**
 * Add the debug heap the tracker's own containers use and register the `heap_report` command.
 *
 * @ghidraAddress 0x10018930
 */
void MemTrackDataInit();

/**
 * Record a heap allocation in the current heap's tracker.
 *
 * @param size The bytes requested.
 * @param type The allocation's name.
 * @param actual The bytes allocated.
 * @param ptr The allocation.
 * @ghidraAddress 0x10018990
 */
void MemTrackAlloc(int size, const char *type, int actual, void *ptr);

/**
 * Forget a heap allocation.
 *
 * @param heap The heap.
 * @param ptr The allocation.
 * @ghidraAddress 0x100189c0
 */
void MemTrackFree(int heap, void *ptr);

/**
 * Record a pool node in the main heap's tracker.
 *
 * @param size The node size in bytes.
 * @param type The node's name.
 * @param ptr The node.
 * @ghidraAddress 0x100189e0
 */
void MemTrackPoolAlloc(int size, const char *type, void *ptr);

/**
 * Forget a pool node.
 *
 * @param ptr The node.
 * @ghidraAddress 0x10018a30
 */
void MemTrackPoolFree(void *ptr);

/**
 * Print a heap's report and its change since the previous one, after the date and time.
 *
 * @param heap The heap.
 * @param minSize The smallest total, in bytes allocated, that prints.
 * @param stream The destination.
 * @ghidraAddress 0x100184c0
 */
void MemTrackReport(int heap, int minSize, PrnStream &stream);

/**
 * Print how many allocations of a heap share each name and size.
 *
 * @param heap The heap.
 * @param stream The destination.
 * @ghidraAddress 0x10018530
 */
void MemTrackReportAllocsByName(int heap, PrnStream &stream);

/**
 * Run the `heap_report` script command.
 *
 * The command is `(heap_report <report|dump|freelist> <heap>...)`. The output goes to the log file
 * `mem_<command>.txt`.
 *
 * @param args The command.
 * @param data The data the command was registered with, unused.
 * @ghidraAddress 0x10018790
 */
void DataHeapReport(DataArray *args, void *data);

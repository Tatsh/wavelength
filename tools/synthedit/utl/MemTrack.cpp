#include "utl/MemTrack.h"

#include <cstdlib>
#include <cstring>
#include <map>
#include <new>

#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/DataFunc.h"
#include "utl/MemMgr.h"
#include "utl/PoolAlloc.h"
#include "utl/common/Pool.h"

namespace {

// Bytes of the heap for the tracker's containers, and its size in words.
const int kDebugHeapBytes = 5000000;
const int kDebugHeapWords = kDebugHeapBytes / 4;

// MemTrackReport() prints totals of at least this many bytes.
const int kReportMinSize = 1000;

// The script command's arguments.
enum HeapReportArg {
    kHeapReportCommand = 1,
    kHeapReportFirstHeap = 2,
};

// The tracker allocates from the C runtime, outside the heaps.
// 0x10018a70
void *TrackMalloc(size_t bytes) {
    return malloc(bytes);
}

// 0x10018a80
void TrackFree(void *mem) {
    free(mem);
}

// Tag that sends a new-expression to TrackMalloc().
struct TrackAlloc {};

} // namespace

void *operator new(size_t size, TrackAlloc /*tag*/) {
    return TrackMalloc(size);
}

// Releases the memory when a constructor run by the matching new-expression throws.
// 0x10018780
void operator delete(void *mem, TrackAlloc /*tag*/) {
    TrackFree(mem);
}

namespace {

// Strict weak order on records by their contents, for the count of identical records.
struct AllocInfoPtrLess {
    bool operator()(const AllocInfo *a, const AllocInfo *b) const {
        return *a < *b;
    }
};

// Print the totals that changed between two reports, current first. The walk stops at the end of
// the previous table, and later totals of the current table are not printed.
// 0x10018550
void PrintStatDiff(PrnStream &stream, BlockStatTable *cur, BlockStatTable *prev) {
    cur->SortByName();
    prev->SortByName();
    const int numPrev = prev->NumStats();
    const int numCur = cur->NumStats();
    int i = 0;
    int j = 0;
    while (i < numCur && j < numPrev) {
        const BlockStat &a = cur->GetBlockStat(i);
        const BlockStat &b = prev->GetBlockStat(j);
        const int order = strcmp(a.mName, b.mName);
        const char *name;
        int deltaAllocs;
        int deltaBytes;
        if (order < 0) {
            name = a.mName;
            deltaAllocs = a.mNumAllocs;
            deltaBytes = a.mSizeReq;
            ++i;
        } else if (order > 0) {
            name = b.mName;
            deltaAllocs = -b.mNumAllocs;
            deltaBytes = -b.mSizeReq;
            ++j;
        } else {
            name = a.mName;
            deltaAllocs = a.mNumAllocs - b.mNumAllocs;
            deltaBytes = a.mSizeReq - b.mSizeReq;
            ++i;
            ++j;
        }
        if (deltaAllocs != 0 || deltaBytes != 0) {
            stream.Printf("  %-50s %8d %8d\n", name, deltaAllocs, deltaBytes);
        }
    }
}

} // namespace

// 0x10039c2c
static int gDebugHeap = -1;

// 0x10039c30
static int gDebugHeapIndex = -1;

// 0x100c3d60
static Pool gAllocInfoPool;

// 0x100c3d70
static MemTrack *gMemTracks[MAX_HEAPS];

// 0x100c3db0
static int gMemTrackActive;

MemTrack::MemTrack(int heap, int *begin, int *end) {
    mHeap = heap;
    mHeapEnd = end;
    mHeapBegin = begin;
    mMemMirrorSize = static_cast<int>(end - begin);
    mMemMirror = static_cast<AllocInfo **>(TrackMalloc(mMemMirrorSize * sizeof(AllocInfo *)));
    mCurNumAllocs = 0;
    mCurByteAllocs = 0;
    mMaxNumAllocs = 0;
    mMaxByteAllocs = 0;
    mTotalAllocs = 0;
    mTotalFrees = 0;
    mReserved2c = 0;
    mCurStat = 0;
    if (mMemMirror) {
        memset(mMemMirror, 0, mMemMirrorSize * sizeof(AllocInfo *));
    }
    for (int i = 0; i < 2; ++i) {
        mStats[i] = NULL;
        mPoolStats[i] = NULL;
    }
}

void MemTrack::SetHeapRange(int *heapBegin, int *heapEnd) {
    ASSERT(mMemMirror);
    ASSERT(heapBegin == mHeapBegin);
    ASSERT(heapEnd < mHeapEnd);
    mHeapEnd = heapEnd;
    mMemMirrorSize = static_cast<int>(heapEnd - heapBegin);
}

void MemTrack::CreateStatTables() {
    for (int i = 0; i < 2; ++i) {
        mStats[i] = new (TrackAlloc()) BlockStatTable(false);
        mPoolStats[i] = new (TrackAlloc()) BlockStatTable(false);
    }
}

void MemTrack::Alloc(int size, const char *type, int actual, void *ptr) {
    ASSERT(type);
    ASSERT(mMemMirror);
    AllocInfo *info = static_cast<AllocInfo *>(gAllocInfoPool.Alloc());
    info->mSizeReq = size;
    info->mSizeActual = actual;
    info->mName = type;
    info->mPoolName = NULL;
    info->mPoolSize = 0;
    AllocInfo **mirror_addr = MirrorSlot(ptr);
    ASSERT(mMemMirror <= mirror_addr && mirror_addr < mMemMirror + mMemMirrorSize);
    ASSERT(*mirror_addr == 0);
    *mirror_addr = info;
    ++mCurNumAllocs;
    mCurByteAllocs += actual;
    ++mTotalAllocs;
    if (mMaxNumAllocs < mCurNumAllocs) {
        mMaxNumAllocs = mCurNumAllocs;
    }
    if (mMaxByteAllocs < mCurByteAllocs) {
        mMaxByteAllocs = mCurByteAllocs;
    }
}

void MemTrack::PoolAlloc(int size, const char *type, void *ptr) {
    ASSERT(type);
    AllocInfo **mirror_addr = MirrorSlot(ptr);
    ASSERT(mMemMirror <= mirror_addr && mirror_addr < mMemMirror + mMemMirrorSize);
    AllocInfo *info = *mirror_addr;
    if (!info) {
        // A node of a chunk the tracker did not see allocated gets a separate record.
        info = static_cast<AllocInfo *>(gAllocInfoPool.Alloc());
        info->mSizeReq = 0;
        info->mSizeActual = 0;
        info->mName = NULL;
        *mirror_addr = info;
    }
    info->mPoolName = type;
    info->mPoolSize = size;
}

void MemTrack::Free(void *ptr, bool pooled) {
    ASSERT(mMemMirror);
    AllocInfo **info = MirrorSlot(ptr);
    ASSERT(mMemMirror <= info && info < mMemMirror + mMemMirrorSize);
    ASSERT(*info);
    const bool norm_info = (*info)->mName != NULL;
    if (pooled) {
        const bool pool_info = (*info)->mPoolName != NULL;
        ASSERT(pool_info);
        if (norm_info) {
            // The heap allocation under the node stays recorded.
            (*info)->mPoolName = NULL;
            (*info)->mPoolSize = 0;
            return;
        }
    } else {
        ASSERT(norm_info);
        --mCurNumAllocs;
        mCurByteAllocs -= (*info)->mSizeActual;
        ++mTotalFrees;
    }
    gAllocInfoPool.Free(*info);
    *info = NULL;
}

int MemTrack::BytesFree() const {
    return static_cast<int>((mHeapEnd - mHeapBegin) * sizeof(int)) - mCurByteAllocs;
}

void MemTrack::ReportAllocsByName(PrnStream &stream) {
    if (!mMemMirror) {
        stream.Printf("*** Mem Tracking is disabled ***\n");
        return;
    }
    if (gDebugHeapIndex == -1) {
        // The quotes are part of the name searched for. The debug heap is never found, and the
        // count is built in the main heap.
        gDebugHeapIndex = MemFindHeap("\"debug\"");
    }
    MemPushHeap(gDebugHeapIndex != -1 ? gDebugHeapIndex : 0);
    {
        std::map<AllocInfo *, int, AllocInfoPtrLess> counts;
        for (AllocInfo **slot = mMemMirror; slot < mMemMirror + mMemMirrorSize; ++slot) {
            if (*slot) {
                ++counts[*slot];
            }
        }
        std::map<AllocInfo *, int, AllocInfoPtrLess>::iterator it;
        for (it = counts.begin(); it != counts.end(); ++it) {
            stream.Printf("( %d ", it->second);
            it->first->Print(stream);
            stream.Printf(")\n");
        }
    }
    MemPopHeap();
}

void MemTrack::Report(PrnStream &stream, int minSize) {
    if (!mMemMirror) {
        stream.Printf("*** Mem Tracking is disabled ***\n");
        return;
    }
    if (!mStats[mCurStat] || !mPoolStats[mCurStat]) {
        stream.Printf("*** Mem Tracking not Inited ***\n");
        return;
    }
    int num_allocs = 0;
    int req_bytes = 0;
    int act_bytes = 0;
    mStats[mCurStat]->Clear();
    mPoolStats[mCurStat]->Clear();
    for (AllocInfo **slot = mMemMirror; slot < mMemMirror + mMemMirrorSize; ++slot) {
        const AllocInfo *info = *slot;
        if (!info) {
            continue;
        }
        if (info->mName) {
            req_bytes += info->mSizeReq;
            act_bytes += info->mSizeActual;
            ++num_allocs;
            mStats[mCurStat]->Update(info->mName, info->mSizeReq, info->mSizeActual);
        }
        if (info->mPoolName) {
            mPoolStats[mCurStat]->Update(info->mPoolName, info->mPoolSize, info->mPoolSize);
        }
    }
    int numBlocks;
    int num_bytes;
    int biggestBlock;
    MemFreeBlockStats(mHeap, &numBlocks, &num_bytes, &biggestBlock);
    stream.Printf("\n*** HEAP REPORT for heap #%d (%d) ***\n",
                  mHeap,
                  static_cast<int>((mHeapEnd - mHeapBegin) * sizeof(int)));
    stream.Printf("  All sizes are in bytes\n");
    stream.Printf("  Num Allocs         = %8d\n", num_allocs);
    stream.Printf("  Bytes Requested    = %8d\n", req_bytes);
    stream.Printf("  Bytes Allocated    = %8d\n", act_bytes);
    stream.Printf("  Peak Num Allocs    = %8d\n", mMaxNumAllocs);
    stream.Printf("  Peak Bytes Alloc'd = %8d\n", mMaxByteAllocs);
    stream.Printf("  Num Free Blocks    = %8d\n", numBlocks);
    stream.Printf("  Biggest Free Block = %8d\n", biggestBlock);
    stream.Printf("  Num Free Bytes     = %8d\n", num_bytes);
    ASSERT(num_allocs == mCurNumAllocs);
    ASSERT(act_bytes == mCurByteAllocs);
    ASSERT(num_bytes == BytesFree());
    BlockStatTable *stats = mStats[mCurStat];
    stats->SortBySize();
    stream.Printf("\n  TYPE                                          NumAllocs  SizeRequest  "
                  "SizeActual\n");
    int i;
    for (i = 0; i < stats->NumStats(); ++i) {
        const BlockStat &stat = stats->GetBlockStat(i);
        if (stat.mSizeActual >= minSize) {
            stream.Printf("  %-45s %9d  %11d  %10d\n",
                          stat.mName,
                          stat.mNumAllocs,
                          stat.mSizeReq,
                          stat.mSizeActual);
        }
    }
    BlockStatTable *poolStats = mPoolStats[mCurStat];
    poolStats->SortBySize();
    stream.Printf("\n  POOL TYPE                                     NumAllocs  SizeRequest  "
                  "SizeActual\n");
    for (i = 0; i < poolStats->NumStats(); ++i) {
        const BlockStat &stat = poolStats->GetBlockStat(i);
        if (stat.mSizeActual >= minSize) {
            stream.Printf("  %-45s %9d  %11d  %10d\n",
                          stat.mName,
                          stat.mNumAllocs,
                          stat.mSizeReq,
                          stat.mSizeActual);
        }
    }
}

void MemTrack::DiffReport(PrnStream &stream) {
    stream.Printf("\n*** Diff Report for heap %d ***\n", mHeap);
    stream.Printf("  MALLOC DELTAS                                     NumAllocs NumBytes\n");
    PrintStatDiff(stream, mStats[mCurStat], mStats[1 - mCurStat]);
    stream.Printf("\n  POOLED DELTAS                                     NumAllocs NumBytes\n");
    PrintStatDiff(stream, mPoolStats[mCurStat], mPoolStats[1 - mCurStat]);
    mCurStat = 1 - mCurStat;
}

bool MemTrackEnabled() {
    return false;
}

void MemTrackInit(int heap, int *begin, int *end) {
    if (!gMemTrackActive) {
        return;
    }
    ASSERT(heap < MAX_HEAPS);
    if (!gMemTracks[heap]) {
        gMemTracks[heap] = new (TrackAlloc()) MemTrack(heap, begin, end);
    } else {
        gMemTracks[heap]->SetHeapRange(begin, end);
    }
    ASSERT(gMemTracks[heap]->InitOK());
}

void MemTrackDataInit() {
    DataRegisterFunc(DataHeapReport, "heap_report", NULL);
    gDebugHeap = MemAddHeap(
        "debug", static_cast<int *>(TrackMalloc(kDebugHeapBytes)), kDebugHeapWords, false);
    for (int i = 0; gMemTracks[i]; ++i) {
        gMemTracks[i]->CreateStatTables();
    }
}

void MemTrackAlloc(int size, const char *type, int actual, void *ptr) {
    MemTrack *track = gMemTracks[MemCurrentHeap()];
    if (track) {
        track->Alloc(size, type, actual, ptr);
    }
}

void MemTrackFree(int heap, void *ptr) {
    MemTrack *track = gMemTracks[heap];
    if (track) {
        track->Free(ptr, false);
    }
}

void MemTrackPoolAlloc(int size, const char *type, void *ptr) {
    ASSERT(gMemTracks[0]);
    gMemTracks[0]->PoolAlloc(size, type, ptr);
}

void MemTrackPoolFree(void *ptr) {
    ASSERT(gMemTracks[0]);
    gMemTracks[0]->Free(ptr, true);
}

void MemTrackReport(int heap, int minSize, PrnStream &stream) {
    MemTrack *track = gMemTracks[heap];
    if (!track) {
        return;
    }
    DateTime now = {};
    GetDateTime(now);
    stream << now << "\n";
    track->Report(stream, minSize);
    track->DiffReport(stream);
}

void MemTrackReportAllocsByName(int heap, PrnStream &stream) {
    MemTrack *track = gMemTracks[heap];
    if (track) {
        track->ReportAllocsByName(stream);
    }
}

void DataHeapReport(DataArray *args, void * /*data*/) {
    const char *command = args->Sym(kHeapReportCommand);
    char logName[56];
    strcpy(logName, FormatString("mem_%s.txt", command));
    const bool usingCD = UsingCD();
    SetUsingCD(false);
    TheDebug.StartLog(logName);
    for (int i = kHeapReportFirstHeap; i < args->Size(); ++i) {
        const int heap = args->Int(i);
        if (!strcmp(command, "report")) {
            MemTrackReport(heap, kReportMinSize, TheDebug);
            PoolReport(TheDebug);
        } else if (!strcmp(command, "dump")) {
            MemTrackReportAllocsByName(heap, TheDebug);
        } else if (!strcmp(command, "freelist")) {
            MemPrintHeap(heap, TheDebug);
        }
    }
    TheDebug.StopLog();
    SetUsingCD(usingCD);
}

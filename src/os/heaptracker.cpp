#include "os/heaptracker.h"

#include <algorithm>
#include <map>
#include <string.h>

#include "os/mem.h"
#include "os/memtrack.h"

namespace {

constexpr int kWordBytes = 4;

// Orders the keys of the map ReportByRecord() counts with.
struct InfoLess {
    bool operator()(const AllocInfo *pFirst, const AllocInfo *pSecond) const {
        return AllocInfo::Less(pFirst, pSecond);
    }
};

} // namespace

// NTSC-U/C: 0x003b2410, PAL: 0x004312d0
static int g_nTrackerReportHeap = -1;

void *HeapTracker::operator new(size_t nSize) {
    return MemTrackBumpAlloc(static_cast<int>(nSize));
}

HeapTracker::HeapTracker(int nHeap, int *pStart, int *pEnd) {
    const int nWords = pEnd - pStart;
    mHeap = nHeap;
    mStart = pStart;
    mEnd = pEnd;
    mSizeWords = nWords;
    mInfos = static_cast<AllocInfo **>(MemTrackBumpAlloc(nWords * kWordBytes));
    mNumAllocs = 0;
    mBytesActual = 0;
    mPeakNumAllocs = 0;
    mPeakBytes = 0;
    mAllocsThisFrame = 0;
    mFreesThisFrame = 0;
    mHistoryIndex = 0;
    mCurrentStats = 0;
    if (mInfos != nullptr) {
        memset(mInfos, 0, mSizeWords * kWordBytes);
    }
    for (int i = 0; i < 2; ++i) {
        mStats[i] = nullptr;
        mPoolStats[i] = nullptr;
    }
}

void HeapTracker::SetRange(int *pStart, int *pEnd) {
    mEnd = pEnd;
    mSizeWords = pEnd - pStart;
}

void HeapTracker::CreateTables() {
    for (int i = 0; i < 2; ++i) {
        mStats[i] = new BlockStatTable(0);
        mPoolStats[i] = new BlockStatTable(0);
    }
}

void HeapTracker::Alloc(int nRequested, const char *pszTag, int nActual, void *pData) {
    AllocInfo *pInfo = static_cast<AllocInfo *>(g_AllocInfoPool.Alloc());
    pInfo->SetSizeRequested(nRequested);
    pInfo->SetTag(pszTag);
    pInfo->SetSizeActual(nActual);
    pInfo->SetPoolTag(nullptr);
    pInfo->SetPoolSize(0);
    pInfo->CaptureStack();
    mInfos[static_cast<int *>(pData) - mStart] = pInfo;
    ++mNumAllocs;
    ++mAllocsThisFrame;
    mBytesActual += nActual;
    mPeakNumAllocs = std::max(mNumAllocs, mPeakNumAllocs);
    mPeakBytes = std::max(mBytesActual, mPeakBytes);
}

void HeapTracker::Realloc(void *pOld, int nRequested, int nActual, void *pNew) {
    const AllocInfo *pInfo = mInfos[static_cast<int *>(pOld) - mStart];
    const char *pszTag = pInfo->GetTag();
    if (nRequested == -1) {
        nRequested = pInfo->GetSizeRequested();
    }
    if (nActual == -1) {
        nActual = pInfo->GetSizeActual();
    }
    Free(pOld, false);
    Alloc(nRequested, pszTag, nActual, pNew);
}

void HeapTracker::PoolAlloc(int nSize, const char *pszTag, void *pData) {
    AllocInfo *&pSlot = mInfos[static_cast<int *>(pData) - mStart];
    AllocInfo *pInfo;
    if (pSlot != nullptr) {
        pInfo = pSlot;
    } else {
        pInfo = static_cast<AllocInfo *>(g_AllocInfoPool.Alloc());
        pInfo->SetSizeRequested(0);
        pInfo->SetSizeActual(0);
        pInfo->SetTag(nullptr);
        pInfo->CaptureStack();
        pSlot = pInfo;
    }
    pInfo->SetPoolSize(nSize);
    pInfo->SetPoolTag(pszTag);
}

void HeapTracker::Free(void *pData, bool bPool) {
    AllocInfo *&pSlot = mInfos[static_cast<int *>(pData) - mStart];
    AllocInfo *pInfo = pSlot;
    if (bPool) {
        if (pInfo->GetTag() != nullptr) {
            pInfo->SetPoolTag(nullptr);
            pSlot->SetPoolSize(0);
            return;
        }
    } else {
        --mNumAllocs;
        ++mFreesThisFrame;
        mBytesActual -= pSlot->GetSizeActual();
    }
    g_AllocInfoPool.Free(pSlot);
    pSlot = nullptr;
}

void HeapTracker::Poll() {
    if (mInfos == nullptr) {
        return;
    }
    ++mHistoryIndex;
    FrameCounts &counts = mHistory[mHistoryIndex % kHistorySize];
    counts.mAllocs = mAllocsThisFrame;
    counts.mFrees = mFreesThisFrame;
    mFreesThisFrame = 0;
    mAllocsThisFrame = 0;
}

int HeapTracker::FreeBytes() {
    return (mEnd - mStart) * kWordBytes - mBytesActual;
}

void HeapTracker::ReportByRecord(PrnStream &stream) {
    if (mInfos == nullptr) {
        stream.Printf("*** Mem Tracking is disabled ***\n");
        return;
    }
    if (g_nTrackerReportHeap == -1) {
        // The quotes are part of the name, so no heap matches.
        g_nTrackerReportHeap = MemFindHeap("\"debug\"");
    }
    MemPushHeap(g_nTrackerReportHeap != -1 ? g_nTrackerReportHeap : 0);
    {
        std::map<AllocInfo *, int, InfoLess> counts;
        for (AllocInfo **ppInfo = mInfos; ppInfo < &mInfos[mSizeWords]; ++ppInfo) {
            if (*ppInfo != nullptr) {
                ++counts[*ppInfo];
            }
        }
        for (const auto &count : counts) {
            stream.Printf("( %d ", count.second);
            count.first->Print(stream);
            stream.Printf(")\n");
        }
    }
    MemPopHeap();
}

void HeapTracker::Report(PrnStream &stream, int nMinSize) {
    if (mInfos == nullptr) {
        stream.Printf("*** Mem Tracking is disabled ***\n");
        return;
    }
    BlockStatTable *pStats = mStats[mCurrentStats];
    if (pStats == nullptr || mPoolStats[mCurrentStats] == nullptr) {
        stream.Printf("*** Mem Tracking not Inited ***\n");
        return;
    }
    pStats->Clear();
    int nAllocs = 0;
    int nRequested = 0;
    int nActual = 0;
    mPoolStats[mCurrentStats]->Clear();
    for (AllocInfo **ppInfo = mInfos; ppInfo < &mInfos[mSizeWords]; ++ppInfo) {
        const AllocInfo *pInfo = *ppInfo;
        if (pInfo == nullptr) {
            continue;
        }
        if (pInfo->GetTag() != nullptr) {
            ++nAllocs;
            nRequested += pInfo->GetSizeRequested();
            nActual += pInfo->GetSizeActual();
            mStats[mCurrentStats]->Update(
                pInfo->GetTag(), pInfo->GetSizeRequested(), pInfo->GetSizeActual());
        }
        if (pInfo->GetPoolTag() != nullptr) {
            mPoolStats[mCurrentStats]->Update(
                pInfo->GetPoolTag(), pInfo->GetPoolSize(), pInfo->GetPoolSize());
        }
    }

    int nFreeBlocks;
    int nFreeBytes;
    int nBiggest;
    MemHeapInfo(mHeap, &nFreeBlocks, &nFreeBytes, &nBiggest);
    stream.Printf("\n*** HEAP REPORT for heap #%d (%d) ***\n",
                  mHeap,
                  static_cast<int>((mEnd - mStart) * kWordBytes));
    stream.Printf("  All sizes are in bytes\n");
    stream.Printf("  Num Allocs         = %8d\n", nAllocs);
    stream.Printf("  Bytes Requested    = %8d\n", nRequested);
    stream.Printf("  Bytes Allocated    = %8d\n", nActual);
    stream.Printf("  Peak Num Allocs    = %8d\n", mPeakNumAllocs);
    stream.Printf("  Peak Bytes Alloc'd = %8d\n", mPeakBytes);
    stream.Printf("  Num Free Blocks    = %8d\n", nFreeBlocks);
    stream.Printf("  Biggest Free Block = %8d\n", nBiggest);
    stream.Printf("  Num Free Bytes     = %8d\n", nFreeBytes);
    (void)FreeBytes(); // Yes, the binary discards this call's result.

    mStats[mCurrentStats]->SortBySize();
    int nCount = mStats[mCurrentStats]->GetCount();
    stream.Printf("\n  TYPE                                          NumAllocs  SizeRequest  "
                  "SizeActual\n");
    for (int i = 0; i < nCount; ++i) {
        const BlockStat *pStat = mStats[mCurrentStats]->GetStat(i);
        if (pStat->mSizeActual < nMinSize) {
            continue;
        }
        stream.Printf("  %-45s %9d  %11d  %10d\n",
                      pStat->mName,
                      pStat->mNumAllocs,
                      pStat->mSizeRequested,
                      pStat->mSizeActual);
    }

    mPoolStats[mCurrentStats]->SortBySize();
    nCount = mPoolStats[mCurrentStats]->GetCount();
    stream.Printf("\n  POOL TYPE                                     NumAllocs  SizeRequest  "
                  "SizeActual\n");
    for (int i = 0; i < nCount; ++i) {
        const BlockStat *pStat = mPoolStats[mCurrentStats]->GetStat(i);
        if (pStat->mSizeActual < nMinSize) {
            continue;
        }
        stream.Printf("  %-45s %9d  %11d  %10d\n",
                      pStat->mName,
                      pStat->mNumAllocs,
                      pStat->mSizeRequested,
                      pStat->mSizeActual);
    }
}

void HeapTracker::ReportDiff(PrnStream &stream) {
    stream.Printf("\n*** Diff Report for heap %d ***\n", mHeap);
    stream.Printf("  MALLOC DELTAS                                     NumAllocs NumBytes\n");
    BlockStatTable::PrintDelta(stream, mStats[mCurrentStats], mStats[1 - mCurrentStats]);
    stream.Printf("\n  POOLED DELTAS                                     NumAllocs NumBytes\n");
    BlockStatTable::PrintDelta(stream, mPoolStats[mCurrentStats], mPoolStats[1 - mCurrentStats]);
    mCurrentStats = 1 - mCurrentStats;
}

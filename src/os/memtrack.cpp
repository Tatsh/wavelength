#include "os/memtrack.h"

#include <stdint.h>
#include <string.h>

#include "os/datetime.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/fixedsizealloc.h"
#include "os/mem.h"
#include "os/string.h"
#include "os/system.h"
#include "script/scriptfunction.h"

namespace {

// The AllocInfo records of the pool.
constexpr int kNumInfos = 350000;

// The heap `debug` the reports allocate from.
constexpr int kDebugHeapBytes = 5000000;

// The development memory above the 32 MB of a retail console.
constexpr uintptr_t kDevMemoryStart = 0x2000000;
constexpr unsigned int kBumpAlign = 16;

// The fewest bytes handed out for which the `report` command lists a tag.
constexpr int kReportMinSize = 1000;

// The script command `heap_report` has the kind of report, then the heaps.
constexpr int kCommandKind = 1;
constexpr int kCommandFirstHeap = 2;

constexpr int kFileNameSize = 64;
constexpr int kWordShift = 2;

} // namespace

HeapTracker *g_apMemTrackHeaps[kMemTrackMaxHeaps];

BlockPool g_AllocInfoPool;

// NTSC-U/C: 0x003b2408, PAL: 0x004312c8
static void *g_pMemTrackInfoMemory = nullptr;

// NTSC-U/C: 0x003b240c, PAL: 0x004312cc
static int g_nMemTrackDebugHeap = -1;

// NTSC-U/C: 0x003b2414, PAL: 0x004312d4
static uintptr_t g_nMemTrackBump = kDevMemoryStart;

bool MemTrackInit() {
    if (kImageSecondWord == 0) {
        return false;
    }
    g_pMemTrackInfoMemory = MemTrackBumpAlloc(kNumInfos * static_cast<int>(sizeof(AllocInfo)));
    g_AllocInfoPool.Init(sizeof(AllocInfo), kNumInfos, g_pMemTrackInfoMemory);
    return true;
}

void MemTrackAddHeap(int nHeap, int *pStart, int *pEnd) {
    if (g_pMemTrackInfoMemory == nullptr) {
        return;
    }
    if (g_apMemTrackHeaps[nHeap] == nullptr) {
        g_apMemTrackHeaps[nHeap] = new HeapTracker(nHeap, pStart, pEnd);
    } else {
        g_apMemTrackHeaps[nHeap]->SetRange(pStart, pEnd);
    }
}

// The script command `heap_report`: write a kind of report of each heap it lists to a log file.
// NTSC-U/C: 0x0029c360, PAL: 0x002a5f80
static void HeapReport(DataArray *pCommand, void * /* pUserData */) {
    const char *pszKind = pCommand->Sym(kCommandKind);
    char szFile[kFileNameSize];
    strcpy(szFile, FormatString("mem_%s.txt", pszKind));
    const bool bUsingCD = UsingCD();
    SetUsingCD(false);
#ifdef VIDEO_STANDARD_PAL
    TheDebug.OpenLog(szFile, false);
#else
    TheDebug.OpenLog(szFile);
#endif
    for (int i = kCommandFirstHeap; i < pCommand->Size(); ++i) {
        const int nHeap = pCommand->Int(i);
        if (strcmp(pszKind, "report") == 0) {
            MemTrackReport(nHeap, kReportMinSize, TheDebug);
            PoolReport(TheDebug);
        } else if (strcmp(pszKind, "dump") == 0) {
            MemTrackReportByRecord(nHeap, TheDebug);
        } else if (strcmp(pszKind, "freelist") == 0) {
            MemPrintHeap(nHeap, TheDebug);
        }
    }
    TheDebug.CloseLog();
    SetUsingCD(bUsingCD);
}

void MemTrackConfigure() {
    ScriptFunction::Register(HeapReport, "heap_report", nullptr);
    g_nMemTrackDebugHeap = MemRegisterHeap("debug",
                                           static_cast<int *>(MemTrackBumpAlloc(kDebugHeapBytes)),
                                           kDebugHeapBytes >> kWordShift,
                                           0);
    for (HeapTracker **ppTracker = g_apMemTrackHeaps; *ppTracker != nullptr; ++ppTracker) {
        (*ppTracker)->CreateTables();
    }
}

void MemTrackAlloc(int nRequested, const char *pszTag, int nActual, void *pData) {
    HeapTracker *pTracker = g_apMemTrackHeaps[MemGetHeap()];
    if (pTracker != nullptr) {
        pTracker->Alloc(nRequested, pszTag, nActual, pData);
    }
}

void MemTrackFree(int nHeap, void *pData) {
    HeapTracker *pTracker = g_apMemTrackHeaps[nHeap];
    if (pTracker != nullptr) {
        pTracker->Free(pData, false);
    }
}

void MemTrackRealloc(int nHeap, void *pOld, int nRequested, int nActual, void *pNew) {
    HeapTracker *pTracker = g_apMemTrackHeaps[nHeap];
    if (pTracker != nullptr) {
        pTracker->Realloc(pOld, nRequested, nActual, pNew);
    }
}

void MemTrackPoolAlloc(int nSize, const char *pszTag, void *pData) {
    g_apMemTrackHeaps[0]->PoolAlloc(nSize, pszTag, pData);
}

void MemTrackPoolFree(void *pData) {
    g_apMemTrackHeaps[0]->Free(pData, true);
}

void MemPoll() {
    if (g_pMemTrackInfoMemory == nullptr) {
        return;
    }
    for (int i = 0; i < kMemTrackMaxHeaps && g_apMemTrackHeaps[i] != nullptr; ++i) {
        g_apMemTrackHeaps[i]->Poll();
    }
}

void *MemTrackBumpAlloc(int nBytes) {
    if (kImageSecondWord == 0) {
        return nullptr;
    }
    const uintptr_t nMemory = g_nMemTrackBump;
    g_nMemTrackBump +=
        (static_cast<unsigned int>(nBytes) + kBumpAlign - 1) / kBumpAlign * kBumpAlign;
    return reinterpret_cast<void *>(nMemory);
}

void MemTrackReport(int nHeap, int nMinSize, PrnStream &stream) {
    if (kImageFirstWord != 0) {
        return;
    }
    HeapTracker *pTracker = g_apMemTrackHeaps[nHeap];
    if (pTracker == nullptr) {
        return;
    }
    DateTime now;
    memset(static_cast<void *>(&now), 0, sizeof(now));
    (void)now.ReadClock(); // Yes, the binary discards the result.
    stream << now << "\n";
    pTracker->Report(stream, nMinSize);
    pTracker->ReportDiff(stream);
}

void MemTrackReportByRecord(int nHeap, PrnStream &stream) {
    HeapTracker *pTracker = g_apMemTrackHeaps[nHeap];
    if (pTracker != nullptr) {
        pTracker->ReportByRecord(stream);
    }
}

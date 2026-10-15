#include <algorithm>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <eekernel.h>

#include "os/cycles.h"
#include "os/debug.h"
#include "os/fixedsizealloc.h"
#include "os/freestore.h"
#include "os/heap.h"
#include "os/mem.h"
#include "os/memtrack.h"
#include "os/system.h"

namespace {

constexpr int kMaxHeaps = 16;
constexpr int kMaxHeapStackDepth = 64;

// The probe that finds the end of the C library's heap, and the end of the memory the game uses.
constexpr size_t kProbeBytes = 0x1000000;
constexpr uintptr_t kMemoryEnd = 0x1f70000;

constexpr int kWordShift = 2;

#ifdef VIDEO_STANDARD_PAL
// The bits of the cycle counter that pick the first word of the main heap, and the bytes the block
// is enlarged by for it.
constexpr unsigned int kStartJitterMask = 0x3f0;
constexpr int kStartJitterSpareBytes = 1024;
#endif

// The alignment of PoolMemAlloc() as a power of two of words, 16 bytes, and the smallest block.
constexpr int kDefaultAlignShift = 2;
constexpr int kMinBlockWords = 2;

// The `heaps_ps2` entries are the name, the size in bytes, and whether the heap moves handles.
constexpr int kHeapEntrySize = 1;
constexpr int kHeapEntryHandles = 2;
constexpr int kHeapEntryWithHandles = 3;

// Blocks up to this size come from the small-block pool.
constexpr int kMaxPoolBlockSize = 128;

} // namespace

// NTSC-U/C: 0x00515340
static Heap g_aHeaps[kMaxHeaps];

// NTSC-U/C: 0x005154c0
static int g_anHeapStack[kMaxHeapStackDepth];

// NTSC-U/C: 0x003b2398, PAL: 0x00431258
static int *g_pMainHeapStart = nullptr;

// NTSC-U/C: 0x003b239c, PAL: 0x0043125c
static int *g_pMainHeapEnd = nullptr;

// NTSC-U/C: 0x003b23a0, PAL: 0x00431260
static int g_nNumHeaps = 0;

// NTSC-U/C: 0x003b23a4, PAL: 0x00431264
static int g_nHeapStackDepth = 0;

// NTSC-U/C: 0x003b23a8, PAL: 0x00431268
static int g_nMemRecursion = 0;

// NTSC-U/C: 0x003b23ac, PAL: 0x0043126c
static int g_nInMemAlloc = 0;

// NTSC-U/C: 0x003b23b0, PAL: 0x00431270
static int g_nMemThread = -1;

int g_nMemTrackingEnabled = 0;
int g_nTrackStl = 0;

// NTSC-U/C: 0x003b23c0, PAL: 0x00431280
static int g_nHeapsConfigured = 0;

// NTSC-U/C: 0x003b242c, PAL: 0x004312ec
static FixedSizeAlloc *g_pSmallBlockPool = nullptr;

int MemGetHeap() {
    return g_anHeapStack[g_nHeapStackDepth - 1];
}

void MemHeapInfo(int nHeap, int *pnBlocks, int *pnFreeBytes, int *pnBiggest) {
    g_aHeaps[nHeap].GetInfo(pnBlocks, pnFreeBytes, pnBiggest);
}

// Take the memory from the end of the C library's heap to kMemoryEnd. The PAL build starts the
// heap a random number of words into the block.
// NTSC-U/C: 0x00299e38, PAL: 0x002a3a38
// Out of line, as in the binary, which reads only the address of the freed probe.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuse-after-free"
__attribute__((noinline)) static void AllocMainHeap(int **ppStart, int **ppEnd) {
#ifdef VIDEO_STANDARD_PAL
    const int nStartWords = static_cast<int>(ReadCycleCount() & kStartJitterMask);
#endif
    void *pProbe = malloc(kProbeBytes);
    const uintptr_t nProbe = reinterpret_cast<uintptr_t>(pProbe);
    free(pProbe);
    const int nBytes = static_cast<int>(kMemoryEnd - nProbe);
#ifdef VIDEO_STANDARD_PAL
    // Yes, the start moves up to 1008 words into a block only 1024 bytes larger.
    int *pBlock = static_cast<int *>(malloc(nBytes + kStartJitterSpareBytes));
    *ppStart = &pBlock[nStartWords];
#else
    *ppStart = static_cast<int *>(malloc(nBytes));
#endif
    *ppEnd = &(*ppStart)[nBytes >> kWordShift];
}
#pragma GCC diagnostic pop

int MemRegisterHeap(const char *pszName, int *pStart, int nSizeWords, int bHandles) {
    g_aHeaps[g_nNumHeaps].Init(pszName, g_nNumHeaps, pStart, nSizeWords, bHandles);
    return g_nNumHeaps++;
}

void MemSetHeapSize(int nHeap, int nSizeWords) {
    g_aHeaps[nHeap].Resize(nSizeWords);
}

void MemPushHeap(int nHeap) {
    g_anHeapStack[g_nHeapStackDepth++] = nHeap;
}

void MemPopHeap() {
    --g_nHeapStackDepth;
}

void MemInit() {
    g_nMemThread = GetThreadId();
    g_nNumHeaps = 0;
    g_nMemRecursion = 0;
    AllocMainHeap(&g_pMainHeapStart, &g_pMainHeapEnd);
    MemRegisterHeap("main", g_pMainHeapStart, g_pMainHeapEnd - g_pMainHeapStart, 0);
    MemPushHeap(0);
    g_nMemTrackingEnabled = MemTrackInit();
    if (g_nMemTrackingEnabled != 0) {
        MemTrackAddHeap(0, g_aHeaps[0].mStart, &g_aHeaps[0].mStart[g_aHeaps[0].mSizeWords]);
    }
}

void MemConfigureHeaps() {
    if (g_nHeapsConfigured != 0) {
        return;
    }
    g_nHeapsConfigured = 1;
    DataArray *pMem = SystemConfig()->FindArray("mem", false);
    DataArray *pHeaps = pMem->FindArray("heaps_ps2", false);
    if (g_nMemTrackingEnabled != 0) {
        pMem->FindBool("enable_tracking", &g_nMemTrackingEnabled, false);
    }
    if (pHeaps != nullptr) {
        int *pNext = g_pMainHeapStart;
        for (int i = 0; i < pHeaps->Size() - 1; ++i) {
            DataArray *pHeap = pHeaps->Array(i + 1);
            const int nSizeWords = pHeap->Int(kHeapEntrySize) >> kWordShift;
            int bHandles = 0;
            if (pHeap->Size() >= kHeapEntryWithHandles) {
                bHandles = pHeap->Int(kHeapEntryHandles) != 0;
            }
            if (i == 0) {
                MemSetHeapSize(0, nSizeWords);
            } else {
                MemRegisterHeap(pHeap->Sym(0), pNext, nSizeWords, bHandles);
            }
            if (g_nMemTrackingEnabled != 0) {
                MemTrackAddHeap(i, g_aHeaps[i].mStart, &g_aHeaps[i].mStart[g_aHeaps[i].mSizeWords]);
            }
            pNext = &pNext[nSizeWords];
        }
        DebugError("MemInit:Leftover memory: %d bytes\n",
                   static_cast<int>((g_pMainHeapEnd - pNext) * sizeof(int)));
    }
    if (g_nMemTrackingEnabled != 0) {
        MemTrackConfigure();
        pMem->FindBool("track_stl", &g_nTrackStl, false);
    }
    FreeStore::Configure(pMem->FindArray("pool", true));
}

void MemTerminate() {
}

void *MemRealloc(void *pBlock, int nBytes) {
    if (pBlock == nullptr) {
        return nullptr;
    }
    if (nBytes == 0) {
        PoolMemFree(pBlock);
        return nullptr;
    }
    const int nSizeWords = (nBytes + 3) >> kWordShift;
    for (int i = 0; i < g_nNumHeaps; ++i) {
        int nActualWords;
        void *pResult = g_aHeaps[i].Realloc(pBlock, nSizeWords, &nActualWords);
        if (pResult != nullptr) {
            if (g_nMemTrackingEnabled != 0) {
                MemTrackRealloc(i, pBlock, nBytes, nActualWords * sizeof(int), pResult);
            }
            return pResult;
        }
    }
    return pBlock;
}

void *PoolMemAlloc(int nSize, const char *pszTag, int nAlign) {
    if (g_nNumHeaps == 0) {
        MemInit();
    }
    int nSizeWords = ((nSize + 3) >> kWordShift) + 1;
    if (static_cast<unsigned int>(nSizeWords) <= 1) {
        nSizeWords = kMinBlockWords;
    }
    g_nInMemAlloc = 1;
    int nAlignShift;
    if (nAlign == 0) {
        nAlignShift = kDefaultAlignShift;
    } else {
        int nLog = 0;
        for (int n = nAlign; n >= 2; n >>= 1) {
            ++nLog;
        }
        nAlignShift = std::max(0, nLog - kWordShift);
    }
    int nActualWords;
    void *pBlock = g_aHeaps[MemGetHeap()].Alloc(nSizeWords, pszTag, nAlignShift, &nActualWords);
    if (g_nMemTrackingEnabled != 0) {
        MemTrackAlloc(nSize, pszTag, nActualWords * sizeof(int), pBlock);
    }
    g_nInMemAlloc = 0;
    return pBlock;
}

void PoolMemFree(void *pBlock) {
    if (pBlock == nullptr) {
        return;
    }
    g_nInMemAlloc = 1;
    for (int i = 0; i < g_nNumHeaps; ++i) {
        if (g_aHeaps[i].Free(pBlock)) {
            if (g_nMemTrackingEnabled != 0) {
                MemTrackFree(i, pBlock);
            }
            break;
        }
    }
    g_nInMemAlloc = 0;
}

int MemFindHeap(const char *pszName) {
    for (int i = 0; i < g_nNumHeaps; ++i) {
        if (strcmp(g_aHeaps[i].mName, pszName) == 0) {
            return i;
        }
    }
    return -1;
}

int MemNumHeaps() {
    return g_nNumHeaps;
}

void MemPrintHeap(int nHeap, PrnStream &stream) {
    g_aHeaps[nHeap].Print(stream, false);
}

void MemCompact(int nHeap, int nByteBudget, bool bStrictBudget) {
    if (nHeap >= 0) {
        g_aHeaps[nHeap].Compact(nByteBudget, bStrictBudget);
    }
}

void *operator new(size_t nSize) {
    return PoolMemAlloc(static_cast<int>(nSize), "new", 0);
}

void operator delete(void *pBlock) noexcept {
    PoolMemFree(pBlock);
}

void *operator new[](size_t nSize) {
    return PoolMemAlloc(static_cast<int>(nSize), "new[]", 0);
}

void operator delete[](void *pBlock) noexcept {
    PoolMemFree(pBlock);
}

void *operator new(size_t nSize, std::align_val_t nAlign) {
    return PoolMemAlloc(static_cast<int>(nSize), "new", static_cast<int>(nAlign));
}

void *operator new[](size_t nSize, std::align_val_t nAlign) {
    return PoolMemAlloc(static_cast<int>(nSize), "new[]", static_cast<int>(nAlign));
}

void operator delete(void *pBlock, std::align_val_t) noexcept {
    PoolMemFree(pBlock);
}

void operator delete[](void *pBlock, std::align_val_t) noexcept {
    PoolMemFree(pBlock);
}

void *PoolAlloc(int nSize, int /* nBlockSize */, const char *pszTag, int nUnused) {
    if (g_pSmallBlockPool == nullptr) {
        g_pSmallBlockPool = new FixedSizeAlloc();
    }
    void *pBlock = g_pSmallBlockPool->Alloc(nSize, nUnused);
    if (g_nMemTrackingEnabled != 0) {
        MemTrackPoolAlloc(nSize, pszTag, pBlock);
    }
    return pBlock;
}

void PoolFree(int nSize, void *pBlock) {
    if (g_nMemTrackingEnabled != 0) {
        MemTrackPoolFree(pBlock);
    }
    g_pSmallBlockPool->Free(pBlock, nSize);
}

void PoolReport(PrnStream &stream) {
    g_pSmallBlockPool->Report(stream);
}

void *MemOrPoolAlloc(int nSize, const char *pszTag) {
    if (nSize == 0) {
        return nullptr;
    }
    if (nSize <= kMaxPoolBlockSize) {
        return PoolAlloc(nSize, nSize, pszTag, 0);
    }
    return PoolMemAlloc(nSize, pszTag, 0);
}

void MemOrPoolFree(int nSize, void *pBlock) {
    if (pBlock == nullptr) {
        return;
    }
    if (nSize <= kMaxPoolBlockSize) {
        PoolFree(nSize, pBlock);
    } else {
        PoolMemFree(pBlock);
    }
}

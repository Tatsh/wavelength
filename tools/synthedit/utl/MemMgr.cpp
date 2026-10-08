#include "utl/MemMgr.h"

#include <cstdlib>
#include <cstring>
#include <new>

#include "os/Debug.h"
#include "os/System.h"
#include "utl/Data.h"
#include "utl/MemTrack.h"
#include "utl/PoolAlloc.h"

namespace {

// Depth of the heap stack.
const int kHeapStackSize = 64;

// The memory all heaps share.
const int kSystemMemoryBytes = 0x2000000;

// Words in a byte count and back.
const int kBytesPerWord = 4;

// Every allocation takes at least its header and one word.
const int kMinAllocWords = 2;

// The default alignment of 16 bytes, as a power of two in words.
const int kDefaultAlignBits = 2;

// A byte alignment of 1 << n is n - 2 in words.
const int kWordAlignBits = 2;

// Positions in a heap's configuration entry.
enum HeapEntryIndex {
    kHeapEntryName = 0,
    kHeapEntrySize = 1,
    kHeapEntryFlag = 2,
};

// The first configuration entry resizes the main heap instead of adding one.
const int kMainHeap = 0;

} // namespace

// 0x100c1130
Heap gHeaps[MAX_HEAPS];

// 0x100c12b0
static int gHeapStack[kHeapStackSize];

// 0x100c13b0
static int *gTotalMemBegin;

// 0x100c13b4
static int *gTotalMemEnd;

// 0x100c13b8
int gNumHeaps;

// 0x100c13bc
static int gHeapStackTop;

// 0x100c13c0
static int gMemReserved;

// 0x100c13c4
static bool gInsideMemFunc;

// 0x100c13c5
bool gMemTracking;

// 0x100c13c6
bool gTrackStl;

// 0x100c13c7
static bool gMemInitDone;

int MemCurrentHeap() {
    return gHeapStack[gHeapStackTop - 1];
}

void MemFreeBlockStats(int heapNum, int *numBlocks, int *freeBytes, int *biggestBlock) {
    ASSERT(heapNum < MAX_HEAPS);
    gHeaps[heapNum].FreeBlockStats(numBlocks, freeBytes, biggestBlock);
}

void MemGetSystemMemory(int **begin, int **end) {
    *begin = static_cast<int *>(malloc(kSystemMemoryBytes));
    *end = *begin + (kSystemMemoryBytes / kBytesPerWord);
}

int MemAddHeap(const char *name, int *start, int sizeWords, bool flag) {
    gHeaps[gNumHeaps].Init(name, gNumHeaps, start, sizeWords, flag);
    return gNumHeaps++;
}

void MemResizeHeap(int heapNum, int sizeWords) {
    gHeaps[heapNum].Resize(sizeWords);
}

void MemPushHeap(int iHeap) {
    ASSERT(!gNumHeaps || (iHeap >= 0 && iHeap < gNumHeaps));
    gHeapStack[gHeapStackTop++] = iHeap;
}

void MemPopHeap() {
    --gHeapStackTop;
}

void MemPreInit() {
    gNumHeaps = 0;
    gMemReserved = 0;
    MemGetSystemMemory(&gTotalMemBegin, &gTotalMemEnd);
    MemAddHeap("main", gTotalMemBegin, static_cast<int>(gTotalMemEnd - gTotalMemBegin), false);
    MemPushHeap(kMainHeap);
    gMemTracking = MemTrackEnabled();
    if (gMemTracking) {
        MemTrackInit(kMainHeap,
                     gHeaps[kMainHeap].Start(),
                     gHeaps[kMainHeap].Start() + gHeaps[kMainHeap].SizeWords());
    }
}

void MemInit() {
    if (gMemInitDone) {
        return;
    }
    gMemInitDone = true;
    DataArray *mem = SystemConfig()->FindArray("mem", false);
    ASSERT(mem);
    DataArray *heaps = mem->FindArray("heaps_pc", false);
    if (gMemTracking) {
        mem->FindBool("enable_tracking", &gMemTracking, false);
    }
    if (heaps) {
        int *heap_start = gTotalMemBegin;
        for (int i = 0; i < heaps->Size() - 1; ++i) {
            DataArray *heap = heaps->Array(i + 1);
            ASSERT(heap);
            const int heap_size = heap->Int(kHeapEntrySize) / kBytesPerWord;
            bool flag = false;
            if (heap->Size() > kHeapEntryFlag) {
                flag = heap->Int(kHeapEntryFlag) != 0;
            }
            ASSERT(heap_start + heap_size < gTotalMemEnd);
            if (i == kMainHeap) {
                MemResizeHeap(kMainHeap, heap_size);
            } else {
                MemAddHeap(heap->Sym(kHeapEntryName), heap_start, heap_size, flag);
            }
            if (gMemTracking) {
                MemTrackInit(i, gHeaps[i].Start(), gHeaps[i].Start() + gHeaps[i].SizeWords());
            }
            heap_start += heap_size;
        }
        DebugPrint("MemInit:Leftover memory: %d bytes\n",
                   static_cast<int>(gTotalMemEnd - heap_start) * kBytesPerWord);
    }
    if (gMemTracking) {
        MemTrackDataInit();
        mem->FindBool("track_stl", &gTrackStl, false);
    }
    PoolAllocInit(mem->FindArray("pool", true));
}

void *MemAlloc(int size, const char *name, int align) {
    if (!gNumHeaps) {
        MemPreInit();
    }
    ASSERT(!gInsideMemFunc);
    gInsideMemFunc = true;
    int sizeWords = ((size + kBytesPerWord - 1) / kBytesPerWord) + 1;
    if (sizeWords < kMinAllocWords) {
        sizeWords = kMinAllocWords;
    }
    int alignBits = kDefaultAlignBits;
    if (align) {
        int log2 = 0;
        while (align > 1) {
            align >>= 1;
            ++log2;
        }
        alignBits = log2 - kWordAlignBits;
        if (alignBits < 0) {
            alignBits = 0;
        }
    }
    int allocatedWords;
    void *mem = gHeaps[MemCurrentHeap()].Alloc(sizeWords, name, alignBits, &allocatedWords);
    if (gMemTracking) {
        MemTrackAlloc(size, name, allocatedWords * kBytesPerWord, mem);
    }
    gInsideMemFunc = false;
    return mem;
}

void MemFree(void *mem) {
    if (!mem) {
        return;
    }
    ASSERT(!gInsideMemFunc);
    gInsideMemFunc = true;
    for (int i = 0; i < gNumHeaps; ++i) {
        if (gHeaps[i].Free(mem)) {
            if (gMemTracking) {
                MemTrackFree(i, mem);
            }
            gInsideMemFunc = false;
            return;
        }
    }
    ASSERT(false);
    gInsideMemFunc = false;
}

int MemFindHeap(const char *name) {
    for (int i = 0; i < gNumHeaps; ++i) {
        if (!strcmp(gHeaps[i].Name(), name)) {
            return i;
        }
    }
    return -1;
}

void MemPrintHeap(int heapNum, PrnStream &stream) {
    gHeaps[heapNum].Print(stream, false);
}

// 0x10010cc0
void *operator new(size_t size) {
    return MemAlloc(static_cast<int>(size), "new", 0);
}

// 0x10010ce0
void operator delete(void *mem) {
    MemFree(mem);
}

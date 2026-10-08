#include "utl/PoolAlloc.h"

#include "os/Debug.h"
#include "utl/ChunkAllocator.h"
#include "utl/MemMgr.h"
#include "utl/MemTrack.h"

namespace {

// Chunks are aligned to 16 bytes.
const int kChunkAlign = 16;

// Larger allocations come from the current heap.
const int kMaxPoolSize = 0x80;

// Words in a byte count and back.
const int kBytesPerWord = 4;

} // namespace

// 0x1003943c
static int gBigHunk = 0x57800;

// 0x10039440
static int gSmallHunk = 0x57800;

// 0x100c18cc
int gPoolChunkBytes;

// 0x100c18d0
static char *gPoolChunkEnd;

// 0x100c18d4
static char *gPoolChunkCur;

// 0x100c18d8
static ChunkAllocator *gChunkAlloc;

void PoolAllocInit(DataArray *config) {
    config->FindInt("big_hunk", &gBigHunk, true);
    config->FindInt("small_hunk", &gSmallHunk, true);
}

void *PoolChunkAlloc(int bytes) {
    gPoolChunkBytes += bytes;
    const int wordBytes = (bytes / kBytesPerWord) * kBytesPerWord;
    char *mem = gPoolChunkCur;
    if (gPoolChunkCur + wordBytes > gPoolChunkEnd) {
        mem = static_cast<char *>(MemAlloc(gBigHunk, "PoolChunk", kChunkAlign));
        gPoolChunkEnd = mem + ((gBigHunk / kBytesPerWord) * kBytesPerWord);
        gBigHunk = gSmallHunk;
    }
    gPoolChunkCur = mem + wordBytes;
    return mem;
}

void *PoolAlloc(int classSize, int reqSize, const char *name, int unused) {
    if (!gChunkAlloc) {
        gChunkAlloc = new ChunkAllocator;
    }
    ASSERT(reqSize == classSize);
    void *mem = gChunkAlloc->Alloc(classSize, unused);
    if (gMemTracking) {
        MemTrackPoolAlloc(classSize, name, mem);
    }
    return mem;
}

void PoolFree(int size, void *mem) {
    if (gMemTracking) {
        MemTrackPoolFree(mem);
    }
    ASSERT(gChunkAlloc);
    gChunkAlloc->Free(mem, size);
}

void PoolReport(PrnStream &stream) {
    ASSERT(gChunkAlloc);
    gChunkAlloc->Print(stream);
}

void *_PoolAlloc(int size, const char *name) {
    if (!size) {
        return NULL;
    }
    if (size > kMaxPoolSize) {
        return MemAlloc(size, name, 0);
    }
    return PoolAlloc(size, size, name, 0);
}

void _PoolFree(int size, void *mem) {
    if (!mem) {
        return;
    }
    if (size > kMaxPoolSize) {
        MemFree(mem);
    } else {
        PoolFree(size, mem);
    }
}

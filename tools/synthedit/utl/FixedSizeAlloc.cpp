#include "utl/FixedSizeAlloc.h"

#include "os/Debug.h"
#include "utl/PoolAlloc.h"

namespace {

// Nodes each chunk holds.
const int kNodesPerChunk = 20;

// Words in a byte count and back.
const int kBytesPerWord = 4;

} // namespace

FixedSizeAlloc::FixedSizeAlloc(int allocSizeWords) {
    mAllocSizeWords = allocSizeWords;
    mNumAllocs = 0;
    mMaxAllocs = 0;
    mNumChunks = 0;
    mFreeList = NULL;
    mNodesPerChunk = kNodesPerChunk;
    ASSERT(mAllocSizeWords != 0);
}

void *FixedSizeAlloc::Alloc() {
    if (!mFreeList) {
        Refill();
    }
    FixedSizeNode *node = mFreeList;
    mFreeList = node->mNext;
    if (++mNumAllocs > mMaxAllocs) {
        mMaxAllocs = mNumAllocs;
    }
    return node;
}

void FixedSizeAlloc::Free(void *mem) {
    FixedSizeNode *node = static_cast<FixedSizeNode *>(mem);
    node->mNext = mFreeList;
    mFreeList = node;
    --mNumAllocs;
}

void FixedSizeAlloc::Refill() {
    ASSERT(mFreeList == 0);
    const int chunkWords = mNodesPerChunk * mAllocSizeWords;
    int *word = static_cast<int *>(RawAlloc(chunkWords * kBytesPerWord));
    ++mNumChunks;
    mFreeList = reinterpret_cast<FixedSizeNode *>(word);
    int *const last = word + (chunkWords - mAllocSizeWords);
    while (word < last) {
        int *const next = word + mAllocSizeWords;
        reinterpret_cast<FixedSizeNode *>(word)->mNext = reinterpret_cast<FixedSizeNode *>(next);
        word = next;
    }
    reinterpret_cast<FixedSizeNode *>(word)->mNext = NULL;
}

void *FixedSizeAlloc::RawAlloc(int bytes) {
    return PoolChunkAlloc(bytes);
}

void FixedSizeAlloc::GetStats(
    int *nodeSize, int *numAllocs, int *maxAllocs, int *capacity, int *wasted) {
    *nodeSize = mAllocSizeWords * kBytesPerWord;
    *numAllocs = mNumAllocs;
    *maxAllocs = mMaxAllocs;
    *capacity = mNodesPerChunk * mNumChunks;
    *wasted = (*capacity - mNumAllocs) * mAllocSizeWords * kBytesPerWord;
}

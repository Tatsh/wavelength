#include "utl/ChunkAllocator.h"

#include <new>

#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "utl/PoolAlloc.h"

namespace {

// Size classes are 16 bytes, four words, apart.
const int kClassShift = 4;
const int kWordsPerClass = 4;

} // namespace

ChunkAllocator::ChunkAllocator() {
    for (int i = 0; i < MAX_FIXED_ALLOCS; ++i) {
        mAllocs[i] = NULL;
    }
}

int ChunkAllocator::SizeToIndex(int size) {
    return (size - 1) >> kClassShift;
}

int ChunkAllocator::IndexToSizeWords(int index) {
    return (index * kWordsPerClass) + kWordsPerClass;
}

void *ChunkAllocator::Alloc(int size, int unused) {
    const int fixedSizeIndex = SizeToIndex(size);
    ASSERT(fixedSizeIndex < MAX_FIXED_ALLOCS);
    if (!mAllocs[fixedSizeIndex]) {
        const int sizeWords = IndexToSizeWords(fixedSizeIndex);
        mAllocs[fixedSizeIndex] =
            new (MemAlloc(sizeof(FixedSizeAlloc), "FixedSizeAlloc", 0)) FixedSizeAlloc(sizeWords);
    }
    return mAllocs[fixedSizeIndex]->Alloc();
}

void ChunkAllocator::Free(void *mem, int size) {
    const int fixedSizeIndex = SizeToIndex(size);
    ASSERT(fixedSizeIndex < MAX_FIXED_ALLOCS);
    ASSERT(mAllocs[fixedSizeIndex]);
    mAllocs[fixedSizeIndex]->Free(mem);
}

void ChunkAllocator::Print(PrnStream &stream) {
    stream.Printf("\n*** POOL REPORT (%d)***\n", gPoolChunkBytes);
    stream.Printf("   NodeSize   NumAllocs  MaxAllocs  Capacity  Wasted\n");
    int totalWaste = 0;
    for (int i = 0; i < MAX_FIXED_ALLOCS; ++i) {
        if (mAllocs[i]) {
            int nodeSize;
            int numAllocs;
            int maxAllocs;
            int capacity;
            int wasted;
            mAllocs[i]->GetStats(&nodeSize, &numAllocs, &maxAllocs, &capacity, &wasted);
            stream.Printf(
                "   %8d  %8d  %8d  %8d  %8d\n", nodeSize, numAllocs, maxAllocs, capacity, wasted);
            totalWaste += wasted;
        }
    }
    stream.Printf("                             Total Waste = %8d\n", totalWaste);
}

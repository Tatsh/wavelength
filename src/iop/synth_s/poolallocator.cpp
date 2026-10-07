#include "synth_s/poolallocator.h"

#include "synth_s/ioputil.h"

namespace {

constexpr int kWordSize = 4;

// The element that starts a given number of bytes after another one.
inline PoolLink *Advance(PoolLink *link, int bytes) {
    return static_cast<PoolLink *>(
        static_cast<void *>(static_cast<unsigned char *>(static_cast<void *>(link)) + bytes));
}

} // namespace

void PoolAllocator::InitWithBlock(int elementSize, int count, void *block) {
    BuildFreeList(elementSize, count, block);
}

void PoolAllocator::Init(int elementSize, int count) {
    const int size = ((elementSize + (kWordSize - 1)) / kWordSize) * kWordSize;
    BuildFreeList(size, count, IopAlloc(size * count));
}

void PoolAllocator::BuildFreeList(int elementSize, int count, void *block) {
    mBlock = static_cast<PoolLink *>(block);
    mElementSize = elementSize;
    mCount = count;
    PoolLink *link = mBlock;
    for (; count >= 2; --count) {
        PoolLink *next = Advance(link, elementSize);
        link->mNext = next;
        link = next;
    }
    link->mNext = nullptr;
    mFree = mBlock;
}

void PoolAllocator::Release() {
    IopFree(mBlock);
}

void *PoolAllocator::Alloc() {
    PoolLink *element = mFree;
    if (element != nullptr) {
        mFree = element->mNext;
        return element;
    }
    return nullptr;
}

void PoolAllocator::Free(void *element) {
    PoolLink *link = static_cast<PoolLink *>(element);
    link->mNext = mFree;
    mFree = link;
}

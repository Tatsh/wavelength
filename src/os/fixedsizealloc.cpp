#include "os/fixedsizealloc.h"

#include <string.h>

namespace {

constexpr int kStoreShift = 4;
constexpr int kStoreNodeWordShift = 2;

} // namespace

int FixedSizeAlloc::StoreIndex(int nSize) {
    return (nSize - 1) >> kStoreShift;
}

int FixedSizeAlloc::StoreNodeWords(int nIndex) {
    return (nIndex + 1) << kStoreNodeWordShift;
}

FixedSizeAlloc::FixedSizeAlloc() {
    memset(static_cast<void *>(this), 0, sizeof(*this));
}

void *FixedSizeAlloc::Alloc(int nSize, int /* nUnused */) {
    const int nIndex = StoreIndex(nSize);
    FreeStore *&pStore = mStores[nIndex];
    if (pStore == nullptr) {
        pStore = new FreeStore(StoreNodeWords(nIndex));
    }
    return pStore->Alloc();
}

void FixedSizeAlloc::Free(void *pBlock, int nSize) {
    mStores[StoreIndex(nSize)]->Free(pBlock);
}

void FixedSizeAlloc::Report(PrnStream &stream) {
    stream.Printf("\n*** POOL REPORT (%d)***\n", FreeStore::sHunkBytes);
    int nTotalWasted = 0;
    stream.Printf("   NodeSize   NumAllocs  MaxAllocs  Capacity  Wasted\n");
    for (int i = 0; i < kNumStores; ++i) {
        if (mStores[i] == nullptr) {
            continue;
        }
        int nNodeBytes;
        int nAllocs;
        int nMaxAllocs;
        int nCapacity;
        int nWasted;
        mStores[i]->GetStats(&nNodeBytes, &nAllocs, &nMaxAllocs, &nCapacity, &nWasted);
        stream.Printf(
            "   %8d  %8d  %8d  %8d  %8d\n", nNodeBytes, nAllocs, nMaxAllocs, nCapacity, nWasted);
        nTotalWasted += nWasted;
    }
    stream.Printf("                             Total Waste = %8d\n", nTotalWasted);
}

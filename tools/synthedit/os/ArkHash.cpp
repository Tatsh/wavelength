#include "os/ArkHash.h"

#include <cstring>

#include "os/Debug.h"
#include "utl/Hash.h"
#include "utl/MemMgr.h"

ArkHash::ArkHash() : mHeap(NULL), mHeapEnd(NULL), mFree(NULL), mTable(NULL), mTableSize(0) {
}

ArkHash::~ArkHash() {
    MemFree(mHeap);
    MemFree(mTable);
}

int ArkHash::GetHashValue(const char *str) {
    int hashIdx = HashString(str, mTableSize);
    ASSERT(hashIdx < mTableSize);
    while (mTable[hashIdx] != NULL) {
        if (strcmp(mTable[hashIdx], str) == 0) {
            return hashIdx;
        }
        ++hashIdx;
        if (hashIdx == mTableSize) {
            hashIdx = 0;
        }
    }
    return -1;
}

void ArkHash::Read(BinStream &stream) {
    MemFree(mHeap);
    MemFree(mTable);
    int heapSize;
    stream.ReadEndian(&heapSize, sizeof(heapSize));
    mHeap = static_cast<char *>(MemAlloc(heapSize, "ArkHash", 0));
    mHeapEnd = mHeap + heapSize;
    mFree = mHeapEnd;
    stream.Read(mHeap, heapSize);
    stream.ReadEndian(&mTableSize, sizeof(mTableSize));
    mTable = static_cast<char **>(MemAlloc(mTableSize * sizeof(char *), "ArkHash", 0));
    stream.Read(mTable, mTableSize * sizeof(char *));
    for (char **slot = mTable; slot != mTable + mTableSize; ++slot) {
        // The stream stores each slot as an offset into the strings.
        int offset;
        memcpy(&offset, slot, sizeof(offset));
        if (offset != 0) {
            *slot = mHeap + offset;
        }
    }
}

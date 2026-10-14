#include "os/blockstattable.h"

#include <algorithm>
#include <string.h>

#include "os/debug.h"
#include "os/memtrack.h"

void *BlockStatTable::operator new(size_t nSize) {
    return MemTrackBumpAlloc(static_cast<int>(nSize));
}

bool BlockStatTable::SizeNotLess(const BlockStat &first, const BlockStat &second) {
    return !(first.mSizeActual < second.mSizeActual);
}

bool BlockStatTable::NameLess(const BlockStat &first, const BlockStat &second) {
    return strcmp(first.mName, second.mName) < 0;
}

BlockStatTable::BlockStatTable(int bSeparateSizes) {
    mSeparateSizes = bSeparateSizes;
    mCapacity = kCapacity;
    mCount = 0;
}

void BlockStatTable::Clear() {
    mCount = 0;
}

BlockStat *BlockStatTable::GetStat(int nIndex) {
    return &mStats[nIndex];
}

void BlockStatTable::Update(const char *pszName, int nRequested, int nActual) {
    int i = 0;
    for (; i < mCount; ++i) {
        BlockStat &stat = mStats[i];
        if (strcmp(stat.mName, pszName) != 0) {
            continue;
        }
        if (stat.mSizeRequested != nRequested && mSeparateSizes != 0) {
            continue;
        }
        if (mSeparateSizes == 0) {
            stat.mSizeRequested += nRequested;
        }
        stat.mSizeActual += nActual;
        ++stat.mNumAllocs;
        stat.mMaxRequested = std::max(nRequested, stat.mMaxRequested);
        return;
    }
    if (i == mCount && i < mCapacity) {
        mStats[mCount].mName = pszName;
        mStats[mCount].mSizeRequested = nRequested;
        mStats[mCount].mMaxRequested = nRequested;
        mStats[mCount].mSizeActual = nActual;
        mStats[mCount].mNumAllocs = 1;
        ++mCount;
    } else {
        DebugWarn("Stack overflow in BlockStatTable!");
    }
}

void BlockStatTable::SortBySize() {
    // The comparison is not strict, as in the binary.
    std::sort(mStats, &mStats[mCount], SizeNotLess);
}

void BlockStatTable::SortByName() {
    std::sort(mStats, &mStats[mCount], NameLess);
}

void BlockStatTable::PrintDelta(PrnStream &stream,
                                BlockStatTable *pCurrent,
                                BlockStatTable *pPrevious) {
    pCurrent->SortByName();
    pPrevious->SortByName();
    const int nCurrent = pCurrent->mCount;
    const int nPrevious = pPrevious->mCount;
    int i = 0;
    int j = 0;
    if (nCurrent <= 0 || nPrevious <= 0) {
        return;
    }
    do {
        const BlockStat *pNew = pCurrent->GetStat(i);
        const BlockStat *pOld = pPrevious->GetStat(j);
        const int nCompare = strcmp(pNew->mName, pOld->mName);
        const char *pszName;
        int nAllocs;
        int nBytes;
        if (nCompare < 0) {
            nBytes = pNew->mSizeRequested;
            pszName = pNew->mName;
            nAllocs = pNew->mNumAllocs;
            ++i;
        } else if (nCompare > 0) {
            nBytes = -pOld->mSizeRequested;
            pszName = pOld->mName;
            nAllocs = -pOld->mNumAllocs;
            ++j;
        } else {
            nBytes = pNew->mSizeRequested - pOld->mSizeRequested;
            ++i;
            ++j;
            pszName = pNew->mName;
            nAllocs = pNew->mNumAllocs - pOld->mNumAllocs;
        }
        if (nAllocs != 0 || nBytes != 0) {
            stream.Printf("  %-50s %8d %8d\n", pszName, nAllocs, nBytes);
        }
    } while (i < nCurrent && j < nPrevious);
}

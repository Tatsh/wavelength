#include "utl/MemStats.h"

#include <algorithm>
#include <cstring>

#include "os/Debug.h"

BlockStatTable::BlockStatTable(bool separateSizes) {
    mMaxStats = kMaxStats;
    mNumStats = 0;
    mSeparateSizes = separateSizes;
}

void BlockStatTable::Clear() {
    mNumStats = 0;
}

BlockStat &BlockStatTable::GetBlockStat(int iStat) {
    ASSERT((0) <= (iStat) && (iStat) < (mNumStats));
    return mStats[iStat];
}

void BlockStatTable::Update(const char *name, int sizeReq, int sizeActual) {
    int i = 0;
    for (; i < mNumStats; ++i) {
        BlockStat &stat = mStats[i];
        if (strcmp(stat.mName, name) != 0) {
            continue;
        }
        if (mSeparateSizes && stat.mSizeReq != sizeReq) {
            continue;
        }
        if (!mSeparateSizes) {
            stat.mSizeReq += sizeReq;
        }
        stat.mSizeActual += sizeActual;
        stat.mMaxSize = std::max(stat.mMaxSize, sizeReq);
        ++stat.mNumAllocs;
        return;
    }
    if (i == mNumStats && mNumStats < mMaxStats) {
        BlockStat &stat = mStats[mNumStats];
        stat.mName = name;
        stat.mSizeReq = sizeReq;
        stat.mMaxSize = sizeReq;
        stat.mSizeActual = sizeActual;
        stat.mNumAllocs = 1;
        ++mNumStats;
        return;
    }
    TheDebug.Fail("Stack overflow in BlockStatTable!");
}

void BlockStatTable::SortBySize() {
    std::sort(mStats, mStats + mNumStats, BlockStat::SizeGreater);
}

void BlockStatTable::SortByName() {
    std::sort(mStats, mStats + mNumStats, BlockStat::NameLess);
}

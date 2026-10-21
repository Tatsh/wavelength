#include "os/heap.h"

#include <limits.h>
#include <stdint.h>

#include <eekernel.h>
#include <eetypes.h>

#include "os/debug.h"
#include "os/memhandle.h"
#include "os/memtrack.h"

namespace {

// Free blocks shorter than this are left inside the neighbouring allocated block.
constexpr int kMinFreeWords = 9;

constexpr int kWordBytes = 4;
constexpr int kWordShift = 2;

// The data of a block is 16-byte aligned with its header in the word before it.
constexpr uintptr_t kDataAlign = 16;

// A handle block of a handle heap starts with three cleared padding words before its header.
constexpr int kHandleHeaderWord = 3;
constexpr int kHandleDataWord = 4;

// The shift from words to the quadwords a quadword copy moves at once.
constexpr int kQuadWordShift = 2;

// The data cache write-back FlushCache() performs.
constexpr int kWritebackDataCache = 0;

inline FreeBlock *BlockAt(int *pWord) {
    return reinterpret_cast<FreeBlock *>(pWord);
}

inline int *WordsOf(FreeBlock *pBlock) {
    return reinterpret_cast<int *>(pBlock);
}

inline BlockHeader *HeaderAt(int *pWord) {
    return reinterpret_cast<BlockHeader *>(pWord);
}

// The address of a word in words, which the alignment of a block is computed from.
inline int AddressWords(const void *pAddress) {
    return static_cast<int>(reinterpret_cast<uintptr_t>(pAddress)) >> kWordShift;
}

} // namespace

// Copy whole quadwords, rounding the count of words down.
// NTSC-U/C: 0x0029afb0, PAL: 0x002a4bd0
static void MoveQuadwords(int *pDest, int *pSource, int nWords) {
    u_long128 *pTo = reinterpret_cast<u_long128 *>(pDest);
    const u_long128 *pFrom = reinterpret_cast<const u_long128 *>(pSource);
    const u_long128 *pEnd = &pTo[nWords >> kQuadWordShift];
    while (pTo < pEnd) {
        *pTo++ = *pFrom++;
    }
}

bool FreeBlock::Merge(FreeBlock *pNext) {
    if (BlockAt(&WordsOf(this)[mSizeWords]) != pNext) {
        return false;
    }
    mNext = pNext->mNext;
    mSizeWords += pNext->mSizeWords;
    return true;
}

void Heap::InsertFreeBlock(FreeBlock *pBlock, int nSizeWords, FreeBlock *pPrev, FreeBlock *pNext) {
    pBlock->mSizeWords = nSizeWords;
    pBlock->mNext = pNext;
    if (pPrev != nullptr) {
        pPrev->mNext = pBlock;
    } else {
        mFreeList = pBlock;
    }
}

void Heap::GetInfo(int *pnBlocks, int *pnFreeBytes, int *pnBiggest) {
    FreeBlock *pBlock = mFreeList;
    *pnBlocks = 0;
    *pnBiggest = 0;
    *pnFreeBytes = 0;
    while (pBlock != nullptr) {
        const int nBytes = pBlock->mSizeWords * kWordBytes;
        if (*pnBiggest < nBytes) {
            *pnBiggest = nBytes;
        }
        pBlock = pBlock->mNext;
        *pnFreeBytes += nBytes;
        ++*pnBlocks;
    }
}

void Heap::Init(const char *pszName, int nNum, int *pStart, int nSizeWords, int bHandles) {
    const uintptr_t nAligned =
        ((reinterpret_cast<uintptr_t>(pStart) - kWordBytes) & ~(kDataAlign - 1)) + kDataAlign;
    int *pAligned = reinterpret_cast<int *>(nAligned);
    const int nSkipWords = pAligned - pStart;
    mNum = nNum;
    mName = pszName;
    nSizeWords -= nSkipWords;
    mHandles = bHandles;
    mSizeWords = nSizeWords;
    mStart = pAligned;
    InsertFreeBlock(BlockAt(pAligned), nSizeWords, nullptr, nullptr);
}

void Heap::Resize(int nSizeWords) {
    FreeBlock *pLast = mFreeList;
    while (pLast->mNext != nullptr) {
        pLast = pLast->mNext;
    }
    pLast->mNext = nullptr;
    pLast->mSizeWords += nSizeWords - mSizeWords;
    mSizeWords = nSizeWords;
}

void *Heap::Alloc(int nSizeWords, const char * /* pszTag */, int nAlignShift, int *pnSizeWords) {
    int nBestSize = INT_MAX;
    int nBestPad = INT_MAX;
    FreeBlock *pBest = nullptr;
    FreeBlock *pBestPrev = nullptr;
    FreeBlock *pPrev = nullptr;
    for (FreeBlock *pBlock = mFreeList; pBlock != nullptr; pBlock = pBlock->mNext) {
        const int nWord = AddressWords(pBlock);
        const int nPad =
            (((nWord + (1 << nAlignShift)) >> nAlignShift) << nAlignShift) - (nWord + 1);
        if (pBlock->mSizeWords >= nSizeWords + nPad && pBlock->mSizeWords < nBestSize) {
            nBestSize = pBlock->mSizeWords;
            nBestPad = nPad;
            pBestPrev = pPrev;
            pBest = pBlock;
        }
        pPrev = pBlock;
    }
    if (pBest == nullptr) {
        DebugPrint(
            "Allocation failure, heap \"%s\", want %d bytes", mName, nSizeWords * kWordBytes);
        int nBlocks;
        int nFreeBytes;
        int nBiggest;
        GetInfo(&nBlocks, &nFreeBytes, &nBiggest);
        // Yes, the binary goes on with a null block.
        DebugWarn(
            "Allocation failure, heap \"%s\", size %d bytes\n   Free Blocks=  %8d\n   Biggest "
            "Block=%8d\n   Free Bytes=   %8d\n",
            mName,
            nSizeWords * kWordBytes,
            nBlocks,
            nBiggest,
            nFreeBytes);
    }

    if (nBestPad >= kMinFreeWords) {
        FreeBlock *pPad = pBest;
        pBest = BlockAt(&WordsOf(pBest)[nBestPad]);
        nBestSize -= nBestPad;
        pBest->mSizeWords = nBestSize;
        pBest->mNext = pPad->mNext;
        InsertFreeBlock(pPad, nBestPad, pBestPrev, pBest);
        pBestPrev = pPad;
        nBestPad = 0;
    }
    int nUsedWords = nSizeWords + nBestPad;
    const int nSpareWords = nBestSize - nUsedWords;
    if (nSpareWords >= kMinFreeWords) {
        InsertFreeBlock(BlockAt(&WordsOf(pBest)[nUsedWords]), nSpareWords, pBestPrev, pBest->mNext);
    } else {
        nUsedWords = nBestSize;
        if (pBestPrev != nullptr) {
            pBestPrev->mNext = pBest->mNext;
        } else {
            mFreeList = pBest->mNext;
        }
    }

    int *pHeaderWord = &WordsOf(pBest)[nBestPad];
    BlockHeader *pHeader = HeaderAt(pHeaderWord);
    pHeader->mPadWords = nBestPad;
    pHeader->mSizeWords = nUsedWords;
    for (int *pWord = &pHeaderWord[-static_cast<int>(pHeader->mPadWords)]; pWord != pHeaderWord;
         ++pWord) {
        *pWord = 0;
    }
    *pnSizeWords = pHeader->mSizeWords;
    return &pHeaderWord[1];
}

void *Heap::Realloc(void *pData, int nSizeWords, int *pnSizeWords) {
    int *pWord = static_cast<int *>(pData);
    if (pWord < mStart || !(pWord < &mStart[mSizeWords])) {
        return nullptr;
    }
    BlockHeader *pHeader = HeaderAt(&pWord[-1]);
    const int nSpareWords = static_cast<int>(pHeader->mSizeWords) -
                            static_cast<int>(pHeader->mPadWords + 1) - nSizeWords;
    if (nSpareWords >= kMinFreeWords) {
        FreeBlock *pPrev = nullptr;
        FreeBlock *pNext = nullptr;
        FindNeighbors(pHeader, &pPrev, &pNext);
        FreeBlock *pTail = BlockAt(&pWord[nSizeWords]);
        InsertFreeBlock(pTail, nSpareWords, pPrev, pNext);
        if (pNext != nullptr) {
            pTail->Merge(pNext);
        }
        pHeader->mSizeWords = pHeader->mSizeWords - nSpareWords;
    }
    *pnSizeWords = pHeader->mSizeWords;
    return pData;
}

void Heap::FindNeighbors(void *pAddress, FreeBlock **ppPrev, FreeBlock **ppNext) {
    FreeBlock *pBlock = mFreeList;
    FreeBlock *pPrev = nullptr;
    while (pBlock != nullptr && static_cast<void *>(pBlock) < pAddress) {
        pPrev = pBlock;
        pBlock = pBlock->mNext;
    }
    *ppNext = pBlock;
    *ppPrev = pPrev;
}

bool Heap::Free(void *pData) {
    int *pWord = static_cast<int *>(pData);
    if (pWord < mStart) {
        return false;
    }
    if (!(pWord < &mStart[mSizeWords])) {
        return false;
    }
    int *pHeaderWord = &pWord[-1];
    FreeBlock *pPrev = nullptr;
    FreeBlock *pNext = nullptr;
    FindNeighbors(pHeaderWord, &pPrev, &pNext);
    const BlockHeader *pHeader = HeaderAt(pHeaderWord);
    FreeBlock *pBlock = BlockAt(&pHeaderWord[-static_cast<int>(pHeader->mPadWords)]);
    InsertFreeBlock(pBlock, pHeader->mSizeWords, pPrev, pNext);
    if (pNext != nullptr) {
        pBlock->Merge(pNext);
    }
    if (pPrev != nullptr) {
        pPrev->Merge(pBlock);
    }
    return true;
}

void Heap::Print(PrnStream &stream, bool bSummary) {
    stream.Printf(" HEAP BLOCKS - %s\n\n", mName);
    int nAllocWords = 0;
    int nAllocCount = 0;
    int *pWord = mStart;
    FreeBlock *pFree = mFreeList;
    stream.Printf(" (START)               Addr: %7x\n", pWord);
    int *pEnd = &mStart[mSizeWords];
    while (pWord < pEnd) {
        int nWords;
        if (pFree != nullptr && BlockAt(pWord) == pFree) {
            if (bSummary) {
                stream.Printf(
                    " (ALLOC) Size:%8d Num: %7d\n", nAllocWords * kWordBytes, nAllocCount);
            }
            nWords = pFree->mSizeWords;
            pFree = pFree->mNext;
            if (bSummary) {
                stream.Printf(" (FREE)  Size:%8d Addr: %7x\n", nWords * kWordBytes, pWord);
            }
            nAllocCount = 0;
            nAllocWords = 0;
        } else {
            int *pHeaderWord = pWord;
            while (*pHeaderWord == 0) {
                ++pHeaderWord;
            }
            ++nAllocCount;
            nWords = HeaderAt(pHeaderWord)->mSizeWords;
            nAllocWords += nWords;
            if (!bSummary) {
                stream.Printf(" (ALLOC) Size:%8d Addr: %7x", nWords * kWordBytes, pWord);
                stream.Printf("\n");
            }
        }
        pWord = &pWord[nWords];
    }
    int nBlocks;
    int nFreeBytes;
    int nBiggest;
    GetInfo(&nBlocks, &nFreeBytes, &nBiggest);
    stream.Printf("   Num Free Blocks =  %8d\n   Total Free Bytes=  %8d\n", nBlocks, nFreeBytes);
}

const char *Heap::Name() const {
    return mName;
}

void Heap::Compact(int nByteBudget, bool bStrict) {
    const int nBudgetWords = nByteBudget >> kWordShift;
    bool bMoved = false;
    FreeBlock *pPrevFree = nullptr;
    FreeBlock *pFree = mFreeList;
    while (true) {
        FreeBlock *pNextFree = pFree->mNext;
        if (pNextFree == nullptr) {
            return;
        }
        const int nFreeWords = pFree->mSizeWords;
        int *pBlock = &WordsOf(pFree)[nFreeWords];
        int nMovedWords = 0;
        bool bBeforeNext;
        while ((bBeforeNext = BlockAt(pBlock) < pNextFree)) {
            const int nWords = HeaderAt(&pBlock[kHandleHeaderWord])->mSizeWords;
            if (bStrict) {
                if (nBudgetWords < nMovedWords + nWords) {
                    break;
                }
            } else if (nBudgetWords < nMovedWords) {
                break;
            }
            HandleBlock *pData = reinterpret_cast<HandleBlock *>(&pBlock[kHandleDataWord]);
            pData->mHandle->SetBlock(
                reinterpret_cast<HandleBlock *>(&pBlock[kHandleDataWord - nFreeWords]));
            if (g_nMemTrackingEnabled != 0) {
                MemTrackRealloc(mNum, pData, -1, -1, pData->mHandle->GetBlock());
            }
            nMovedWords += nWords;
            pBlock = &pBlock[nWords];
            bMoved = true;
        }
        if (!bMoved) {
            pPrevFree = pFree;
            pFree = pNextFree;
            continue;
        }

        MoveQuadwords(WordsOf(pFree), &WordsOf(pFree)[pFree->mSizeWords], nMovedWords);
        FreeBlock *pNewFree = BlockAt(&WordsOf(pFree)[nMovedWords]);
        if (bBeforeNext) {
            pNewFree->mNext = pNextFree;
            pNewFree->mSizeWords = nFreeWords;
        } else {
            pNewFree->mNext = pNextFree->mNext;
            pNewFree->mSizeWords = &WordsOf(pNextFree)[pNextFree->mSizeWords] - WordsOf(pNewFree);
        }
        if (pPrevFree != nullptr) {
            pPrevFree->mNext = pNewFree;
        } else {
            mFreeList = pNewFree;
        }
        FlushCache(kWritebackDataCache);
        return;
    }
}

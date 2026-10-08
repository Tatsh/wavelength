#include "utl/Heap.h"

#include <climits>

#include "os/Debug.h"

namespace {

// Allocations start on a 16-byte boundary.
const int kHeapAlignMask = 15;

// A leftover run of this many words or fewer stays with the allocation.
const int kMinSplitWords = 8;

// Words in a byte count and back.
const int kBytesPerWord = 4;

} // namespace

void Heap::InsertFreeBlock(FreeBlock *iBlock,
                           int sizeWords,
                           FreeBlock *iPrevBlock,
                           FreeBlock *iNextBlock) {
    ASSERT((iBlock != iPrevBlock) && (iBlock != iNextBlock));
    iBlock->mNext = iNextBlock;
    iBlock->mSizeWords = sizeWords;
    if (iPrevBlock) {
        iPrevBlock->mNext = iBlock;
    } else {
        mFreeList = iBlock;
    }
}

void Heap::FreeBlockStats(int *numBlocks, int *freeBytes, int *biggestBlock) {
    *numBlocks = 0;
    *biggestBlock = 0;
    *freeBytes = 0;
    for (FreeBlock *block = mFreeList; block; block = block->mNext) {
        const int bytes = block->mSizeWords * kBytesPerWord;
        if (*biggestBlock < bytes) {
            *biggestBlock = bytes;
        }
        *freeBytes += bytes;
        ++*numBlocks;
    }
}

void Heap::Init(const char *name, int num, int *start, int sizeWords, bool flag) {
    mNum = num;
    mStart = start;
    mFlag = flag;
    mName = name;
    const unsigned long aligned =
        ((reinterpret_cast<unsigned long>(start) - sizeof(int)) & ~kHeapAlignMask) +
        kHeapAlignMask + 1;
    mStart = reinterpret_cast<int *>(aligned);
    mSizeWords = sizeWords - static_cast<int>(mStart - start);
    if (!mStart) {
        TheDebug.Fail("mName = %s mSizeWords = %i.\n", name, mSizeWords);
    }
    InsertFreeBlock(reinterpret_cast<FreeBlock *>(mStart), mSizeWords, NULL, NULL);
}

void Heap::Resize(int sizeWords) {
    FreeBlock *freeBlock = mFreeList;
    while (freeBlock->mNext) {
        freeBlock = freeBlock->mNext;
    }
    ASSERT(freeBlock->EndAddr() == mStart + mSizeWords);
    const int new_fb_size = freeBlock->mSizeWords - mSizeWords + sizeWords;
    ASSERT(new_fb_size > 0);
    freeBlock->mSizeWords = new_fb_size;
    freeBlock->mNext = NULL;
    mSizeWords = sizeWords;
}

void *Heap::Alloc(int sizeWords, const char *name, int alignBits, int *allocatedWords) {
    FreeBlock *best = NULL;
    FreeBlock *bestPrev = NULL;
    FreeBlock *prev = NULL;
    int bestSize = INT_MAX;
    int padWords = INT_MAX;
    if (mFreeList) {
        const int alignMask = (1 << alignBits) - 1;
        for (FreeBlock *block = mFreeList; block; block = block->mNext) {
            // The allocation follows the header word. Its alignment is measured from the word
            // after the run's first word.
            const int dataWord = static_cast<int>(reinterpret_cast<unsigned long>(block) >> 2) + 1;
            const int pad = (((alignMask + dataWord) >> alignBits) << alignBits) - dataWord;
            if (block->mSizeWords >= sizeWords + pad && block->mSizeWords < bestSize) {
                padWords = pad;
                bestSize = block->mSizeWords;
                best = block;
                bestPrev = prev;
            }
            prev = block;
        }
    }
    if (!best) {
        TheDebug.Printf(
            "Allocation failure, heap \"%s\", want %d bytes", mName, sizeWords * kBytesPerWord);
        int numBlocks;
        int freeBytes;
        int biggestBlock;
        FreeBlockStats(&numBlocks, &freeBytes, &biggestBlock);
        TheDebug.Fail("Allocation failure, heap \"%s\", size %d bytes\n   Free Blocks=  %8d\n   "
                      "Biggest Block=%8d\n   Free Bytes=   %8d\n",
                      mName,
                      sizeWords * kBytesPerWord,
                      numBlocks,
                      biggestBlock,
                      freeBytes);
    }
    int *run = reinterpret_cast<int *>(best);
    if (padWords > kMinSplitWords) {
        // The alignment gap becomes a separate free run in front of the allocation.
        FreeBlock *rest = reinterpret_cast<FreeBlock *>(run + padWords);
        rest->mNext = best->mNext;
        bestSize -= padWords;
        rest->mSizeWords = bestSize;
        InsertFreeBlock(best, padWords, bestPrev, rest);
        bestPrev = best;
        run = reinterpret_cast<int *>(rest);
        padWords = 0;
    }
    FreeBlock *taken = reinterpret_cast<FreeBlock *>(run);
    int usedWords = padWords + sizeWords;
    const int leftover = bestSize - usedWords;
    if (leftover > kMinSplitWords) {
        InsertFreeBlock(
            reinterpret_cast<FreeBlock *>(run + usedWords), leftover, bestPrev, taken->mNext);
    } else {
        usedWords = bestSize;
        if (bestPrev) {
            bestPrev->mNext = taken->mNext;
        } else {
            mFreeList = taken->mNext;
        }
    }
    AllocHeader *header = reinterpret_cast<AllocHeader *>(run + padWords);
    header->mPadWords = padWords;
    header->mSizeWords = usedWords;
    for (int *pad = run; pad != reinterpret_cast<int *>(header); ++pad) {
        *pad = 0;
    }
    *allocatedWords = header->mSizeWords;
    return header + 1;
}

void Heap::FindNeighbors(int *addr, FreeBlock **prev, FreeBlock **next) {
    FreeBlock *before = NULL;
    FreeBlock *block = mFreeList;
    while (block && reinterpret_cast<int *>(block) < addr) {
        before = block;
        block = block->mNext;
    }
    *next = block;
    *prev = before;
}

bool Heap::Free(void *mem) {
    int *const data = static_cast<int *>(mem);
    if (data < mStart || data >= mStart + mSizeWords) {
        return false;
    }
    AllocHeader *header = reinterpret_cast<AllocHeader *>(data) - 1;
    FreeBlock *prev = NULL;
    FreeBlock *next = NULL;
    FindNeighbors(reinterpret_cast<int *>(header), &prev, &next);
    FreeBlock *block =
        reinterpret_cast<FreeBlock *>(reinterpret_cast<int *>(header) - header->mPadWords);
    InsertFreeBlock(block, header->mSizeWords, prev, next);
    if (next) {
        block->Coalesce(next);
    }
    if (prev) {
        prev->Coalesce(block);
    }
    return true;
}

void Heap::Print(PrnStream &stream, bool summary) {
    stream.Printf(" HEAP BLOCKS - %s\n\n", mName);
    int *word = mStart;
    FreeBlock *freeBlock = mFreeList;
    stream.Printf(" (START)               Addr: %7x\n", word);
    int *const end = mStart + mSizeWords;
    int numAllocs = 0;
    int allocWords = 0;
    while (word < end) {
        int runWords;
        if (freeBlock && word == reinterpret_cast<int *>(freeBlock)) {
            if (summary) {
                stream.Printf(
                    " (ALLOC) Size:%8d Num: %7d\n", allocWords * kBytesPerWord, numAllocs);
            }
            runWords = freeBlock->mSizeWords;
            freeBlock = freeBlock->mNext;
            if (summary) {
                stream.Printf(" (FREE)  Size:%8d Addr: %7x\n", runWords * kBytesPerWord, word);
            }
            numAllocs = 0;
            allocWords = 0;
        } else {
            // Alignment padding is zero, and a header never is.
            const int *headerWord = word;
            while (*headerWord == 0) {
                ++headerWord;
            }
            runWords = reinterpret_cast<const AllocHeader *>(headerWord)->mSizeWords;
            ++numAllocs;
            allocWords += runWords;
            if (!summary) {
                stream.Printf(" (ALLOC) Size:%8d Addr: %7x", runWords * kBytesPerWord, word);
                stream.Printf("\n");
            }
        }
        word += runWords;
    }
    int numBlocks;
    int freeBytes;
    int biggestBlock;
    FreeBlockStats(&numBlocks, &freeBytes, &biggestBlock);
    stream.Printf("   Num Free Blocks =  %8d\n   Total Free Bytes=  %8d\n", numBlocks, freeBytes);
}

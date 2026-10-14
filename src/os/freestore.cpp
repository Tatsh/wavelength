#include "os/freestore.h"

namespace {

constexpr int kNodesPerChunk = 20;
constexpr int kWordBytes = 4;
constexpr int kWordShift = 2;
constexpr int kHunkAlign = 16;
// The size of a hunk until FreeStore::Configure() reads the configuration.
constexpr int kDefaultHunkSize = 358400;

} // namespace

// NTSC-U/C: 0x003b2418
static int g_nBigHunkSize = kDefaultHunkSize;

// NTSC-U/C: 0x003b241c
static int g_nSmallHunkSize = kDefaultHunkSize;

int FreeStore::sHunkBytes;

// NTSC-U/C: 0x003b2424
static char *g_pHunkEnd;

// NTSC-U/C: 0x003b2428
static char *g_pHunkCursor;

void FreeStore::Configure(DataArray *pConfig) {
    pConfig->FindInt("big_hunk", &g_nBigHunkSize, true);
    pConfig->FindInt("small_hunk", &g_nSmallHunkSize, true);
}

void *FreeStore::AllocHunk(int nBytes) {
    sHunkBytes += nBytes;
    const int nAligned = (nBytes >> kWordShift) << kWordShift;
    if (g_pHunkEnd < &g_pHunkCursor[nAligned]) {
        g_pHunkCursor = static_cast<char *>(PoolMemAlloc(g_nBigHunkSize, "PoolChunk", kHunkAlign));
        const int nHunkBytes = (g_nBigHunkSize >> kWordShift) << kWordShift;
        g_nBigHunkSize = g_nSmallHunkSize;
        g_pHunkEnd = &g_pHunkCursor[nHunkBytes];
    }
    char *pBytes = g_pHunkCursor;
    g_pHunkCursor = &g_pHunkCursor[nAligned];
    return pBytes;
}

FreeStore::FreeStore(int nNodeWords) {
    mNodeWords = nNodeWords;
    mNodesPerChunk = kNodesPerChunk;
    mNumAllocs = 0;
    mMaxAllocs = 0;
    mNumChunks = 0;
    mFree = nullptr;
}

void *FreeStore::Alloc() {
    if (mFree == nullptr) {
        Grow();
    }
    Node *pNode = mFree;
    ++mNumAllocs;
    mFree = pNode->mNext;
    if (mMaxAllocs < mNumAllocs) {
        mMaxAllocs = mNumAllocs;
    }
    return pNode;
}

void FreeStore::Free(void *pNode) {
    Node *pFree = static_cast<Node *>(pNode);
    pFree->mNext = mFree;
    mFree = pFree;
    --mNumAllocs;
}

void FreeStore::Grow() {
    const int nWords = mNodeWords * mNodesPerChunk;
    int *pNode = static_cast<int *>(AllocChunk(nWords * kWordBytes));
    int *pLast = &pNode[nWords - mNodeWords];
    mFree = reinterpret_cast<Node *>(pNode);
    ++mNumChunks;
    while (pNode < pLast) {
        int *pNext = &pNode[mNodeWords];
        reinterpret_cast<Node *>(pNode)->mNext = reinterpret_cast<Node *>(pNext);
        pNode = pNext;
    }
    reinterpret_cast<Node *>(pNode)->mNext = nullptr;
}

void *FreeStore::AllocChunk(int nBytes) {
    return AllocHunk(nBytes);
}

void FreeStore::GetStats(
    int *pnNodeBytes, int *pnAllocs, int *pnMaxAllocs, int *pnCapacity, int *pnWasted) {
    *pnNodeBytes = mNodeWords * kWordBytes;
    *pnAllocs = mNumAllocs;
    *pnMaxAllocs = mMaxAllocs;
    const int nCapacity = mNumChunks * mNodesPerChunk;
    *pnCapacity = nCapacity;
    *pnWasted = (nCapacity - mNumAllocs) * mNodeWords * kWordBytes;
}

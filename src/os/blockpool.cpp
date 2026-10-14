#include "os/blockpool.h"

void BlockPool::Init(int nBlockSize, int nNumBlocks, void *pMemory) {
    Setup(nBlockSize, nNumBlocks, pMemory);
}

BlockPool::Node *BlockPool::Setup(int nBlockSize, int nNumBlocks, void *pMemory) {
    char *pBlock = static_cast<char *>(pMemory);
    mMemory = static_cast<Node *>(pMemory);
    mBlockSize = nBlockSize;
    mNumBlocks = nNumBlocks;
    for (int n = nNumBlocks; n >= 2; --n) {
        char *pNext = &pBlock[nBlockSize];
        reinterpret_cast<Node *>(pBlock)->mNext = reinterpret_cast<Node *>(pNext);
        pBlock = pNext;
    }
    reinterpret_cast<Node *>(pBlock)->mNext = nullptr;
    mFree = mMemory;
    return mFree;
}

void *BlockPool::Alloc() {
    Node *pBlock = mFree;
    if (pBlock == nullptr) {
        return nullptr;
    }
    mFree = pBlock->mNext;
    return pBlock;
}

void BlockPool::Free(void *pBlock) {
    Node *pNode = static_cast<Node *>(pBlock);
    pNode->mNext = mFree;
    mFree = pNode;
}

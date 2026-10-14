#include "os/memhandle.h"

namespace {

// A block is the data rounded up to 16 bytes, after a 16-byte header.
constexpr int kBlockAlign = 16;
constexpr int kBlockOverhead = 32;

inline int BlockBytes(int nBytes) {
    return ((nBytes - 1) & ~(kBlockAlign - 1)) + kBlockOverhead;
}

} // namespace

MemHandle::MemHandle(HandleBlock *pBlock) {
    mBlock = pBlock;
    mBlock->mHandle = this;
    mBlock->mLockCount = 0;
}

void *MemHandle::Lock() {
    ++mBlock->mLockCount;
    return &mBlock[1];
}

void MemHandle::Unlock() {
    --mBlock->mLockCount;
}

MemHandle *MemHandle::Alloc(int nBytes, const char *pszTag) {
    HandleBlock *pBlock = static_cast<HandleBlock *>(PoolMemAlloc(BlockBytes(nBytes), pszTag, 0));
    return new MemHandle(pBlock);
}

void *MemHandle::Resize(int nBytes) {
    return MemRealloc(mBlock, BlockBytes(nBytes));
}

void MemHandle::Free(MemHandle *pHandle) {
    if (pHandle != nullptr) {
        PoolMemFree(pHandle->mBlock);
        delete pHandle;
    }
}

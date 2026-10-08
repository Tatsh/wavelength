#include "os/BlockMgr.h"

#include "os/CDReader.h"
#include "os/Debug.h"

BlockMgr TheBlockMgr;

void BlockMgr::Init() {
    gCurrBuffNum = 0;
    mSectorsPerBlock = kBlockSize / kSectorSize;
    ASSERT(kBlockSize % kSectorSize == 0);
    mBlocks.resize(kNumBlockBuffers, NULL);
    mLoadingBlock = NULL;
    for (unsigned int i = 0; i < mBlocks.size(); ++i) {
        mBlocks[i] = new Block;
    }
}

void BlockMgr::GetAssociatedBlocks(
    __int64 offset, int bytes, int &firstBlock, int &numBlocks, int &blockSize) {
    blockSize = kBlockSize;
    firstBlock = static_cast<int>(offset / kBlockSize);
    const int overflow = static_cast<int>(offset % kBlockSize) - kBlockSize + bytes;
    if (overflow > 0) {
        numBlocks = overflow / kBlockSize + 1;
        if (overflow % kBlockSize != 0) {
            ++numBlocks;
        }
    } else {
        numBlocks = 1;
    }
}

void BlockMgr::KillBlockRequests(ArkFile *file) {
    std::list<BlockRequest>::iterator request;
    for (request = mRequests.begin(); request != mRequests.end();) {
        std::list<AsyncTask> &tasks = request->mTasks;
        std::list<AsyncTask>::iterator task;
        for (task = tasks.begin(); task != tasks.end();) {
            if (task->mFile == file) {
                task = tasks.erase(task);
            } else {
                ++task;
            }
        }
        if (tasks.size() == 0 &&
            (mLoadingBlock == NULL || request->mBlockNum != mLoadingBlock->mBlockNum)) {
            request = mRequests.erase(request);
        } else {
            ++request;
        }
    }
}

char *BlockMgr::GetBlockData(int blockNum) {
    Block *block = FindBlock(blockNum);
    if (block == NULL || block == mLoadingBlock) {
        return NULL;
    }
    block->UpdateTimestamp();
    return block->mBuffer;
}

void BlockMgr::AddRequest(const AsyncTask &task) {
    std::list<BlockRequest>::iterator request;
    for (request = mRequests.begin(); request != mRequests.end(); ++request) {
        if (request->mBlockNum == task.mBlockNum) {
            request->mTasks.insert(request->mTasks.end(), task);
            return;
        }
        if (request->mBlockNum > task.mBlockNum) {
            mRequests.insert(request, BlockRequest(task.mBlockNum, task));
            return;
        }
    }
    mRequests.insert(mRequests.end(), BlockRequest(task.mBlockNum, task));
}

void BlockMgr::Poll() {
    if (mLoadingBlock != NULL && CDReadDone()) {
        const int error = CDGetError();
        if (error != 0) {
            TheDebug.Printf(" CD READING ERROR!!!  %d", error);
            if (CDReadSectors(mLoadingBlock->mBlockNum * mSectorsPerBlock,
                              mSectorsPerBlock,
                              mLoadingBlock->mBuffer) == 0) {
                TheDebug.Fail(" CAN'T RECOVER FROM CD ERROR, BAILING OUT\n");
            }
            return;
        }
        mLoadingBlock->UpdateTimestamp();
        std::list<BlockRequest>::iterator request = mRequests.begin();
        while (request != mRequests.end() && request->mBlockNum != mLoadingBlock->mBlockNum) {
            ++request;
        }
        ASSERT(request != mRequests.end());
        mLoadingBlock = NULL;
        std::list<AsyncTask>::iterator task;
        for (task = request->mTasks.begin(); task != request->mTasks.end(); ++task) {
            task->FillData();
        }
        mRequests.erase(request);
    }
    if (mLoadingBlock != NULL || mRequests.size() == 0) {
        return;
    }
    Block *block = GetOldestBlock();
    if (block == NULL) {
        TheDebug.Fail(" ERROR: No available blocks for loading");
        return;
    }
    const int blockNum = mRequests.front().mBlockNum;
    mLoadingBlock = block;
    block->mBlockNum = blockNum;
    mLoadingBlock->UpdateTimestamp();
    CDReadSectors(mSectorsPerBlock * blockNum, mSectorsPerBlock, mLoadingBlock->mBuffer);
}

Block *BlockMgr::FindBlock(int blockNum) {
    for (unsigned int i = 0; i < mBlocks.size(); ++i) {
        if (mBlocks[i]->mBlockNum == blockNum) {
            return mBlocks[i];
        }
    }
    return NULL;
}

Block *BlockMgr::GetOldestBlock() {
    int oldest = Block::CurrentTimestamp();
    Block *result = NULL;
    for (unsigned int i = 0; i < mBlocks.size(); ++i) {
        if (mBlocks[i]->mTimestamp < oldest) {
            result = mBlocks[i];
            oldest = mBlocks[i]->mTimestamp;
        }
    }
    return result;
}

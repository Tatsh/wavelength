#include "os/blockmgr.h"

#include <cstring>

#include <libcdvd.h>

#include "os/debug.h"

namespace {

// Blocks of the cache.
constexpr int kBlockCount = 8;

// Bytes of a block.
constexpr int kBlockBytes = 0x10000;

// Bytes of a disc sector.
constexpr int kSectorBytes = 2048;
constexpr int kSectorShift = 11;

// The block number of an empty block.
constexpr int kNoBlock = -1;

// Retries of a failed disc read.
constexpr unsigned char kReadTryCount = 3;

// NTSC-U/C: 0x00492680
alignas(64) char gBlockBuffers[kBlockCount][kBlockBytes];

// The buffer each new block takes in turn.
// NTSC-U/C: 0x003b2300
char *gBlockBufferTable[kBlockCount] = {gBlockBuffers[0],
                                        gBlockBuffers[1],
                                        gBlockBuffers[2],
                                        gBlockBuffers[3],
                                        gBlockBuffers[4],
                                        gBlockBuffers[5],
                                        gBlockBuffers[6],
                                        gBlockBuffers[7]};

// NTSC-U/C: 0x003b2320
int gBlockSize = kBlockBytes;

// NTSC-U/C: 0x003b2324
int gNextBuffer;

// NTSC-U/C: 0x003b2328
int gBlockTime;

} // namespace

// NTSC-U/C: 0x00290280, PAL: 0x00299c48 (static initialiser)
// NTSC-U/C: 0x00290338, PAL: 0x00299d00 (constructor call)
// NTSC-U/C: 0x00290358, PAL: 0x00299d20 (destructor call)
BlockMgr TheBlockMgr;

int Block::NextBuffer() {
    return gNextBuffer++;
}

Block::Block() : mBlockNum(kNoBlock) {
    mBuffer = gBlockBufferTable[NextBuffer()];
    Touch();
}

int Block::CurrentTime() {
    return gBlockTime;
}

void Block::Touch() {
    mTimestamp = ++gBlockTime;
}

AsyncTask::AsyncTask(ArkFile *pFile, char *pBuffer, int nBlock, int nStart, int nEnd)
    : mBlock(nBlock), mStart(nStart), mEnd(nEnd), mBuffer(pBuffer), mFile(pFile) {
}

bool AsyncTask::TryComplete() {
    const char *pData = TheBlockMgr.GetBlockData(mBlock);
    if (pData == nullptr) {
        return false;
    }
    memcpy(mBuffer, pData + mStart, mEnd - mStart);
    mFile->TaskDone(mEnd - mStart);
    return true;
}

BlockRequest::BlockRequest(int nBlock, const AsyncTask &task) : mBlockNum(nBlock) {
    mTasks.push_back(task);
}

void BlockMgr::Init() {
    mSectorsPerBlock = gBlockSize / kSectorBytes;
    gNextBuffer = 0;
    mBlocks.resize(kBlockCount, nullptr);
    mLoading = nullptr;
    for (unsigned int i = 0; i < mBlocks.size(); ++i) {
        mBlocks[i] = new Block();
    }
}

void BlockMgr::GetBlockSpan(
    long long nOffset, int nBytes, int *pnFirst, int *pnCount, int *pnBlockSize) {
    *pnBlockSize = gBlockSize;
    *pnFirst = static_cast<int>(nOffset / gBlockSize);
    const int nRest = nBytes - gBlockSize + static_cast<int>(nOffset % gBlockSize);
    if (nRest <= 0) {
        *pnCount = 1;
        return;
    }
    *pnCount = nRest / gBlockSize + 1;
    if (nRest % gBlockSize != 0) {
        *pnCount = nRest / gBlockSize + 2;
    }
}

void BlockMgr::KillBlockRequests(ArkFile *pFile) {
    for (auto request = mRequests.begin(); request != mRequests.end();) {
        for (auto task = request->mTasks.begin(); task != request->mTasks.end();) {
            if (task->mFile == pFile) {
                task = request->mTasks.erase(task);
            } else {
                ++task;
            }
        }
        if (request->mTasks.size() == 0 &&
            (mLoading == nullptr || request->mBlockNum != mLoading->mBlockNum)) {
            request = mRequests.erase(request);
        } else {
            ++request;
        }
    }
}

int BlockMgr::SectorToByte(int nSector) {
    return nSector << kSectorShift;
}

char *BlockMgr::GetBlockData(int nBlock) {
    Block *pBlock = FindBlock(nBlock);
    if (pBlock == nullptr || pBlock == mLoading) {
        return nullptr;
    }
    pBlock->Touch();
    return pBlock->mBuffer;
}

void BlockMgr::AddRequest(const AsyncTask &task) {
    for (auto request = mRequests.begin(); request != mRequests.end(); ++request) {
        if (request->mBlockNum == task.mBlock) {
            request->mTasks.push_back(task);
            return;
        }
        if (task.mBlock < request->mBlockNum) {
            mRequests.insert(request, BlockRequest(task.mBlock, task));
            return;
        }
    }
    mRequests.push_back(BlockRequest(task.mBlock, task));
}

void BlockMgr::Poll() {
    if (mLoading != nullptr) {
        if (!ReadDone()) {
            return;
        }
        const int nError = ReadError();
        if (nError != 0) {
            DebugPrint(" CD READING ERROR!!!  %d", nError);
            if (!Read(
                    mLoading->mBlockNum * mSectorsPerBlock, mSectorsPerBlock, mLoading->mBuffer)) {
                DebugWarn(" CAN'T RECOVER FROM CD ERROR, BAILING OUT\n");
            }
            return;
        }
        mLoading->Touch();
        auto request = mRequests.begin();
        while (request != mRequests.end() && request->mBlockNum != mLoading->mBlockNum) {
            ++request;
        }
        mLoading = nullptr;
        for (auto &task : request->mTasks) {
            task.TryComplete(); // Yes, the binary drops a task whose block is gone again.
        }
        mRequests.erase(request);
    }
    if (mRequests.size() == 0) {
        return;
    }
    Block *pBlock = FindOldestBlock();
    if (pBlock == nullptr) {
        DebugWarn(" ERROR: No available blocks for loading");
        return;
    }
    const int nBlock = mRequests.front().mBlockNum;
    mLoading = pBlock;
    pBlock->mBlockNum = nBlock;
    mLoading->Touch();
    Read(nBlock * mSectorsPerBlock, mSectorsPerBlock, mLoading->mBuffer);
}

Block *BlockMgr::FindBlock(int nBlock) {
    for (unsigned int i = 0; i < mBlocks.size(); ++i) {
        if (mBlocks[i]->mBlockNum == nBlock) {
            return mBlocks[i];
        }
    }
    return nullptr;
}

Block *BlockMgr::FindOldestBlock() {
    int nOldest = Block::CurrentTime();
    Block *pOldest = nullptr;
    for (unsigned int i = 0; i < mBlocks.size(); ++i) {
        if (mBlocks[i]->mTimestamp < nOldest) {
            pOldest = mBlocks[i];
            nOldest = mBlocks[i]->mTimestamp;
        }
    }
    return pOldest;
}

bool BlockMgr::ReadDone() {
    return sceCdSync(SCECdNonblock) == 0;
}

int BlockMgr::ReadError() {
    return sceCdGetError();
}

bool BlockMgr::Read(int nSector, int nCount, void *pBuffer) {
    sceCdRMode mode;
    mode.trycount = kReadTryCount;
    mode.spindlctrl = 0;
    mode.datapattern = 0;
    mode.pad = 0;
    return sceCdRead(nSector, nCount, pBuffer, &mode) != 0;
}

#include "os/streambuffer.h"

#include <string.h>

#include "os/debug.h"

namespace {

// The buffer is allocated in whole quadwords.
constexpr unsigned int kQuadwordSize = 16;

constexpr int kDefaultMargin = 16;

} // namespace

int g_nAddWasEntered = 0;

inline void StreamBuffer::CheckHeads() {
    if (!(mWriteHead < mEndOfBuffer)) {
        DebugPrint("StreamBuffer.cpp - mReadHead = %x mWriteHead = %x mEndOfBuffer = %x "
                   "gAddWasEntered = %i.\n",
                   mReadHead,
                   mWriteHead,
                   mEndOfBuffer,
                   g_nAddWasEntered);
    }
}

StreamBuffer::StreamBuffer(int nSize) {
    mBuffer = nullptr;
    mReadHead = nullptr;
    mEndOfBuffer = nullptr;
    mReadRunEnd = nullptr;
    mInputDone = 0;
    mMargin = kDefaultMargin;
    mSize = nSize;
    // The binary rounds the allocation down to whole quadwords but clears mSize bytes.
    mBuffer =
        std::make_unique<char[]>(static_cast<unsigned int>(nSize) / kQuadwordSize * kQuadwordSize);
    memset(mBuffer.get(), 0, mSize);
    mReadHead = mBuffer.get();
    mWriteHead = mBuffer.get();
    mEndOfBuffer = &mBuffer[mSize];
}

StreamBuffer::~StreamBuffer() = default;

bool StreamBuffer::IsEof() {
    if (!(mWriteHead < mEndOfBuffer)) {
        DebugPrint("ERROR!!! Thread problem in soft fx. returning Eof() true.\n");
        return true;
    }
    return mReadHead == mWriteHead && mInputDone == 1;
}

bool StreamBuffer::IsEmpty() {
    CheckHeads();
    return mReadHead == mWriteHead;
}

char *StreamBuffer::AcquireReadRun(int nMaxBytes, int *pnBytes) {
    CheckHeads();
    Poll();
    CheckHeads();
    if (IsEof()) {
        return nullptr;
    }
    CheckHeads();
    CheckHeads();
    if (IsEmpty()) {
        return nullptr;
    }
    CheckHeads();
    unsigned int nAvailable;
    if (mReadHead < mWriteHead) {
        nAvailable = mWriteHead - mReadHead;
    } else {
        nAvailable = mEndOfBuffer - mReadHead;
    }
    CheckHeads();
    *pnBytes = nAvailable < static_cast<unsigned int>(nMaxBytes) ? nAvailable : nMaxBytes;
    CheckHeads();
    mReadRunEnd = &mReadHead[*pnBytes];
    CheckHeads();
    CheckHeads();
    return mReadHead;
}

void StreamBuffer::CommitReadRun(char **ppRun) {
    CheckHeads();
    if (*ppRun == nullptr) {
        return;
    }
    CheckHeads();
    CheckHeads();
    mReadHead = mReadRunEnd;
    if (mReadRunEnd == mEndOfBuffer) {
        mReadHead = mBuffer.get();
    }
    mReadRunEnd = nullptr;
    CheckHeads();
    Poll();
    *ppRun = nullptr;
    CheckHeads();
}

void StreamBuffer::Add(const char *pData, int nBytes, int bLast) {
    g_nAddWasEntered = 1;
    CheckHeads();
    if (static_cast<unsigned int>(EmptySpaceForFilling()) < static_cast<unsigned int>(nBytes)) {
        DebugPrint(
            "iNumDataBytes [%i] > EmptySpaceForFilling() [%i]!\n", nBytes, EmptySpaceForFilling());
    }
    (void)EmptySpaceForFilling(); // Yes, the binary discards this call's result.
    CheckHeads();
    mInputDone = bLast;
    if (pData == nullptr || nBytes == 0) {
        return; // The binary returns with g_nAddWasEntered still set.
    }
    const unsigned int nTail = mEndOfBuffer - mWriteHead;
    if (mWriteHead < mReadHead || nTail >= static_cast<unsigned int>(nBytes)) {
        memcpy(mWriteHead, pData, nBytes);
        mWriteHead = &mWriteHead[nBytes];
        if (mEndOfBuffer < mWriteHead) {
            DebugPrint("mWriteHead [%i] > mEndOfBuffer [%i]. (1)\n", mWriteHead, mEndOfBuffer);
        }
        if (mWriteHead == mEndOfBuffer) {
            mWriteHead = mBuffer.get();
        }
    } else {
        CheckHeads();
        const int nRest = nBytes - nTail;
        memcpy(mWriteHead, pData, nTail);
        memcpy(mBuffer.get(), &pData[nTail], nRest);
        mWriteHead = &mBuffer[nRest];
        CheckHeads();
    }
    CheckHeads();
    g_nAddWasEntered = 0;
}

int StreamBuffer::EmptySpaceForFilling() {
    int bInputDone;
    return mSize - (BytesAvailable(&bInputDone) + mMargin);
}

int StreamBuffer::BytesAvailable(int *pbInputDone) {
    int nBytes = mWriteHead - mReadHead;
    if (mWriteHead < mReadHead) {
        nBytes += mSize;
    }
    *pbInputDone = mInputDone;
    CheckHeads();
    return nBytes;
}

void StreamBuffer::Rewind() {
    mReadHead = mBuffer.get();
    mWriteHead = mBuffer.get();
}

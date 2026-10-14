#include "os/chunkstream.h"

#include <algorithm>
#include <cstring>

#include "os/debug.h"
#include "os/loadfile.h"
#include "os/mem.h"
#include "os/system.h"

namespace {

// File::New() modes of a reader and a writer. A writer creates or truncates the file.
constexpr int kOpenRead = 1;
constexpr int kOpenWrite = 0x602;
constexpr int kNoOpenFlags = 0;

// Origin of File::Seek() at the start of the file.
constexpr int kSeekStart = 0;

// Block buffers a writer gathers into, in chunks.
constexpr int kWriteBufferChunks = 2;

// A writer's gathered block grows by this much beyond the bytes it must take.
constexpr int kWriteGrowth = 8192;

// mBuffer before the first block.
constexpr int kNoBuffer = -1;

// The buffer a writer gathers into.
constexpr int kFirstBuffer = 0;

} // namespace

const char *g_szChunkMark = "chunk";

BinStream &EndChunk(BinStream &stream) {
    stream.Mark(g_szChunkMark);
    return stream;
}

ChunkStream::ChunkStream(
    const char *pszFile, Mode eMode, int nChunkSize, bool bCompressed, bool bLittleEndian)
    : BinStream(bLittleEndian), mMode(eMode), mHeader(bCompressed) {
    mCompressed = bCompressed;
    mChunkSize = nChunkSize;
    mBlockReady = 0;
    mPosition = 0;
    mStallMs = 0.0F;
    mBuffers[0] = nullptr;
    mBuffers[1] = nullptr;
    mWork = nullptr;
    mData = nullptr;
    mBuffer = kNoBuffer;
    mFile = File::New(pszFile, eMode == kModeRead ? kOpenRead : kOpenWrite, kNoOpenFlags);
    mFail = mFile == nullptr;
    if (mFile == nullptr) {
        return;
    }
    if (eMode == kModeWrite) {
        mFile->Write(&mHeader, sizeof(mHeader));
        mBufferSize = mChunkSize * kWriteBufferChunks;
        mWork = static_cast<char *>(PoolMemAlloc(mBufferSize, "ChunkStream", 0));
        mBuffer = kFirstBuffer;
        return;
    }
    mHeaderPending = 1;
    mHeader.mMagic = 0;
    mFile->ReadAsync(&mHeader, sizeof(mHeader));
}

ChunkStream::~ChunkStream() {
    if (mFail == 0 && mMode == kModeWrite) {
        FlushBlock(true);
        mFile->Seek(0, kSeekStart);
        mFile->Write(&mHeader, sizeof(mHeader));
    }
    delete mFile;
    PoolMemFree(mWork);
    PoolMemFree(mBuffers[0]);
    PoolMemFree(mBuffers[1]);
}

void ChunkStream::Read(void *pData, int nBytes) {
    memcpy(pData, mData + mPosition, nBytes);
    mPosition += nBytes;
}

void ChunkStream::Write(const void *pData, int nBytes) {
    if (mPosition + nBytes >= mBufferSize) {
        mBufferSize = mPosition + nBytes + kWriteGrowth;
        char *pWork = static_cast<char *>(PoolMemAlloc(mBufferSize, "ChunkStream", 0));
        memcpy(pWork, mWork, mPosition);
        PoolMemFree(mWork);
        mWork = pWork;
    }
    memcpy(mWork + mPosition, pData, nBytes);
    mPosition += nBytes;
}

void ChunkStream::Seek(int /* nOffset */, SeekType /* eFrom */) {
    DebugWarn("Can't seek on chunkstream");
}

int ChunkStream::Tell() {
    DebugWarn("Can't tell on chunkstream");
    return 0;
}

bool ChunkStream::Eof() {
    if (mHeaderPending != 0) {
        int nRead;
        if (!mFile->ReadDone(&nRead)) {
            return true;
        }
        mHeaderPending = 0;
        if (mHeader.mMagic == kMagicCompressed) {
            mCompressed = 1;
        } else if (mHeader.mMagic == kMagicPlain) {
            mCompressed = 0;
        } else {
            // A file without the header is one plain block.
            mCompressed = 0;
            mHeader.mBlockCount = 1;
            mHeader.mHeaderSize = 0;
            const int nSize = mFile->Size();
            mHeader.mBlockSizes[0] = nSize;
            mHeader.mMaxBlockSize = nSize;
        }
        int nLargest = 0;
        for (int i = 0; i < mHeader.mBlockCount; ++i) {
            nLargest = std::max(nLargest, mHeader.mBlockSizes[i]);
        }
        mBufferSize = nLargest;
        if (mHeader.mBlockCount >= kWriteBufferChunks) {
            mBuffers[0] = static_cast<char *>(PoolMemAlloc(nLargest, "ChunkStream", 0));
        }
        mBuffers[1] = static_cast<char *>(PoolMemAlloc(nLargest, "ChunkStream", 0));
        if (mCompressed != 0) {
            mWork = static_cast<char *>(PoolMemAlloc(mHeader.mMaxBlockSize, "ChunkStream", 0));
        }
        // mBlock starts on the entry before the first size, which mPosition has already reached.
        mBlock = &mHeader.mMaxBlockSize;
        mPosition = mHeader.mMaxBlockSize;
        mBlockEnd = &mHeader.mBlockSizes[mHeader.mBlockCount];
        mBuffer = kFirstBuffer;
        mFile->Seek(mHeader.mHeaderSize, kSeekStart);
        mFile->ReadAsync(mBuffers[mBuffer == 0 ? 1 : 0], mBlock[1]);
    }

    if (mPosition < *mBlock) {
        return false;
    }
    mBlockReady = 0;
    if (mBlock + 1 == mBlockEnd) {
        return true;
    }
    int nRead;
    if (!mFile->ReadDone(&nRead)) {
        if (mStallMs == 0.0F) {
            mStallMs = SystemMs();
        }
        return true;
    }
    mBlockReady = 1;
    mBuffer = mBuffer == 0;
    mStallMs = 0.0F;
    ++mBlock;
    mPosition = 0;
    if (mBlock + 1 != mBlockEnd) {
        mFile->ReadAsync(mBuffers[mBuffer == 0 ? 1 : 0], mBlock[1]);
    }
    DecodeBlock();
    return false;
}

bool ChunkStream::Fail() {
    return mFail;
}

void ChunkStream::Mark(const char *pszMark) {
    if (strcmp(pszMark, g_szChunkMark) == 0) {
        FlushBlock(false);
    }
}

void ChunkStream::FlushBlock(bool bLast) {
    mHeader.mBlockSizes[mHeader.mBlockCount - 1] = mPosition;
    mHeader.mMaxBlockSize = std::max(mPosition, mHeader.mMaxBlockSize);
    if (mPosition < mChunkSize && !bLast) {
        return;
    }
    mHeader.mBlockSizes[mHeader.mBlockCount - 1] =
        WriteBlock(mHeader.mBlockSizes[mHeader.mBlockCount - 1]);
    mPosition = 0;
    if (!bLast) {
        mHeader.mBlockSizes[mHeader.mBlockCount++] = 0;
    }
}

int ChunkStream::WriteBlock(int nBytes) {
    if (mCompressed != 0) {
        nBytes = GzipCompressRamToRam(mWork, nBytes, mWork);
    }
    if (mFile->Write(mWork, nBytes) != nBytes) {
        mFail = 1;
    }
    return nBytes;
}

void ChunkStream::DecodeBlock() {
    if (mCompressed != 0) {
        *mBlock = GzipDecompressRamToRam(mBuffers[mBuffer], *mBlock, mWork);
        mData = mWork;
    } else {
        mData = mBuffers[mBuffer];
    }
}

ChunkStream::Header::Header(bool bCompressed) {
    mMagic = bCompressed ? kMagicCompressed : kMagicPlain;
    mHeaderSize = sizeof(Header);
    mBlockCount = 1;
    mMaxBlockSize = 0;
    mBlockSizes[0] = 0;
}

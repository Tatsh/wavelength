#include "os/chunkstream.h"

#include <cstring>

#include "os/mem.h"

namespace {

// File::New() modes of a reader and a writer. A writer creates or truncates the file.
constexpr int kOpenRead = 1;
constexpr int kOpenWrite = 0x602;
constexpr int kNoOpenFlags = 0;

// Origin of File::Seek() at the start of the file.
constexpr int kSeekStart = 0;

// Block buffers a writer gathers into, in chunks.
constexpr int kWriteBufferChunks = 2;

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

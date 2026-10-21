#include "synth/lazyfilestream.h"

#include "os/mem.h"

LazyFileStream::LazyFileStream(const String &filename, int nBufferSize)
    : mMoreData(1), mPosition(0), mFile(nullptr), mCurBuffer(0), mBufferSize(nBufferSize),
      mBytesAvailable(-1) {
    mFile.reset(File::New(filename.c_str(), File::kModeRead, 0));
    for (unsigned int i = 0; i < kNumBuffers; ++i) {
        mBuffers[i] = static_cast<char *>(PoolMemAlloc(nBufferSize, "LazyFileStream buf", 0));
    }
    StartRead();
}

LazyFileStream::~LazyFileStream() {
    mFile.reset();
    for (unsigned int i = 0; i < kNumBuffers; ++i) {
        PoolMemFree(mBuffers[i]);
    }
}

bool LazyFileStream::Eof() const {
    return mMoreData == 0;
}

void LazyFileStream::StartRead() {
    (void)mFile->ReadAsync(mBuffers[mCurBuffer], mBufferSize); // Yes, the result is discarded.
}

bool LazyFileStream::ReadDone() {
    int nBytes;
    const bool bDone = mFile->ReadDone(&nBytes);
    if (bDone) {
        mBytesAvailable = nBytes;
    }
    return bDone;
}

char *LazyFileStream::GetData(int *pnPosition, int *pnBytesAvailable) {
    char *pBuffer = mBuffers[mCurBuffer];
    *pnPosition = mPosition;
    *pnBytesAvailable = mBytesAvailable;
    mPosition += mBytesAvailable;
    if (mFile->Eof()) {
        mMoreData = 0;
    } else {
        mBytesAvailable = -1;
        ++mCurBuffer;
        if (mCurBuffer >= kNumBuffers) {
            mCurBuffer = 0;
        }
        StartRead();
    }
    return pBuffer;
}

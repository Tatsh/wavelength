#include "synth/LazyFileStream.h"

#include "os/Debug.h"
#include "utl/MemMgr.h"

LazyFileStream::LazyFileStream(const String &filename, int bufferSize)
    : mMoreData(true), mPosition(0), mFile(NULL), mCurBuffer(0), mBufferSize(bufferSize),
      mBytesAvailable(-1) {
    ASSERT(kNumBuffers > 1);
    mFile = NewFile(filename.c_str(), FILE_OPEN_READ, 0);
    ASSERT(mFile);
    for (int i = 0; i < kNumBuffers; ++i) {
        mBuffers[i] = static_cast<char *>(MemAlloc(bufferSize, "LazyFileStream buf", 0));
    }
    StartRead();
}

LazyFileStream::~LazyFileStream() {
    ASSERT(mFile);
    delete mFile;
    for (int i = 0; i < kNumBuffers; ++i) {
        MemFree(mBuffers[i]);
    }
}

bool LazyFileStream::Eof() const {
    return !mMoreData;
}

void LazyFileStream::StartRead() {
    ASSERT(mFile);
    char *buffer = mBuffers[mCurBuffer];
    bool ok = mFile->ReadAsync(buffer, mBufferSize);
    ASSERT(ok);
}

bool LazyFileStream::ReadDone() {
    int bytes;
    bool done = mFile->ReadDone(&bytes);
    if (done) {
        mBytesAvailable = bytes;
    }
    return done;
}

char *LazyFileStream::GetData(int *position, int *bytesAvailable) {
    ASSERT(mBytesAvailable >= 0);
    char *buffer = mBuffers[mCurBuffer];
    *position = mPosition;
    *bytesAvailable = mBytesAvailable;
    mPosition += mBytesAvailable;
    if (!mFile->Eof()) {
        mBytesAvailable = -1;
        ++mCurBuffer;
        if (mCurBuffer >= kNumBuffers) {
            mCurBuffer = 0;
        }
        StartRead();
    } else {
        mMoreData = false;
    }
    return buffer;
}

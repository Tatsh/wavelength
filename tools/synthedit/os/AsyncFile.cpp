#include "os/AsyncFile.h"

#include <cstring>

#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/Data.h"
#include "utl/MemMgr.h"

namespace {

const int kNoHandle = -1;
const int kBufferAlign = 0x40;
const int kSizeTrailerBytes = 4;

} // namespace

int gAsyncBufSize = 0x2000;

void AsyncFileInit() {
    SystemConfig()->FindArray("file", true)->FindInt("buf_size", &gAsyncBufSize, false);
}

AsyncFile::AsyncFile(const char *name, int mode)
    : mMode(0), mHandle(kNoHandle), mTell(0), mOffset(0), mReadStarted(false), mFail(false),
      mBuffer(NULL), mData(NULL), mBytesLeft(0), mBytesRead(0) {
    mBuffer = static_cast<char *>(MemAlloc(gAsyncBufSize, "AsyncFile buf", kBufferAlign));
    Open(name, mode);
}

AsyncFile::~AsyncFile() {
    Close();
    MemFree(mBuffer);
}

void AsyncFile::Open(const char *name, int mode) {
    ASSERT(mHandle == -1);
    mMode = mode;
    ASSERT((mMode & (FILE_OPEN_READ | FILE_OPEN_WRITE)) != (FILE_OPEN_READ | FILE_OPEN_WRITE));
    FileMakeLocalPath(mFilename, name);
    mSize = _Open();
    if (!mFail && strcmp(FileGetExt(name), "gz") == 0 && (mMode & FILE_OPEN_READ) &&
        mSize >= kSizeTrailerBytes) {
        mTell = mSize - kSizeTrailerBytes;
        _SeekToTell();
        _ReadAsync(&mUCSize, sizeof(mUCSize));
        while (!_ReadDone()) {
        }
        mTell = 0;
        _SeekToTell();
    } else {
        mUCSize = 0;
    }
    if (mMode & FILE_OPEN_READ) {
        mOffset = gAsyncBufSize;
        Flush();
    }
}

void AsyncFile::Close() {
    if (mMode & FILE_OPEN_WRITE) {
        Flush();
    }
    if (mHandle > kNoHandle) {
        _Close();
    }
    mHandle = kNoHandle;
    mMode = 0;
    mTell = 0;
    mFail = false;
    mOffset = 0;
    mData = NULL;
    mBytesLeft = 0;
    mBytesRead = 0;
}

int AsyncFile::Read(void *data, int bytes) {
    ReadAsync(data, bytes);
    if (mFail) {
        return 0;
    }
    while (!ReadDone(&bytes)) {
    }
    return bytes;
}

bool AsyncFile::ReadAsync(void *data, int bytes) {
    ASSERT(mMode & FILE_OPEN_READ);
    if (mFail) {
        return false;
    }
    if (mTell + bytes > mSize) {
        bytes = mSize - mTell;
    }
    mBytesLeft = bytes;
    mData = static_cast<char *>(data);
    mBytesRead = 0;
    ReadDone(&bytes); // Yes, the binary discards the result.
    return true;
}

bool AsyncFile::ReadDone(int *bytes) {
    if (mFail) {
        *bytes = 0;
        return true;
    }
    if (mBytesLeft != 0) {
        if (!_ReadDone()) {
            *bytes = mBytesRead;
            return false;
        }
        if (mOffset + mBytesLeft > gAsyncBufSize) {
            const int copied = gAsyncBufSize - mOffset;
            memcpy(mData, &mBuffer[mOffset], copied);
            mBytesRead += copied;
            mOffset = gAsyncBufSize;
            mBytesLeft -= copied;
            mData += copied;
            mTell += copied;
            Flush();
            return false;
        }
        memcpy(mData, &mBuffer[mOffset], mBytesLeft);
        mBytesRead += mBytesLeft;
        mOffset += mBytesLeft;
        mTell += mBytesLeft;
        mBytesLeft = 0;
    }
    *bytes = mBytesRead;
    return true;
}

int AsyncFile::Write(const void *data, int bytes) {
    ASSERT(mMode & FILE_OPEN_WRITE);
    if (mFail) {
        return 0;
    }
    const char *source = static_cast<const char *>(data);
    int left = bytes;
    while (mOffset + left > gAsyncBufSize) {
        const int copied = gAsyncBufSize - mOffset;
        memcpy(&mBuffer[mOffset], source, copied);
        left -= copied;
        mOffset = gAsyncBufSize;
        source += copied;
        mTell += copied;
        Flush();
    }
    memcpy(&mBuffer[mOffset], source, left);
    mOffset += left;
    mTell += left;
    if (mTell > mSize) {
        mSize = mTell;
    }
    return bytes;
}

int AsyncFile::Seek(int offset, int origin) {
    if (mFail) {
        return mTell;
    }
    if (mMode & FILE_OPEN_WRITE) {
        Flush();
    }
    switch (origin) {
    case BinStream::kSeekCurrent:
        mTell += offset;
        break;
    case BinStream::kSeekBegin:
        mTell = offset;
        break;
    case BinStream::kSeekEnd:
        mTell = mSize + offset;
        break;
    }
    if (mTell < 0) {
        mTell = 0;
    } else if (mTell > mSize) {
        mTell = mSize;
    }
    _SeekToTell();
    if (mMode & FILE_OPEN_READ) {
        mOffset = gAsyncBufSize;
        Flush();
    }
    return mTell;
}

int AsyncFile::Tell() {
    return mTell;
}

void AsyncFile::Flush() {
    if (mFail) {
        return;
    }
    if (mMode & FILE_OPEN_WRITE) {
        _Write(mBuffer, mOffset);
    } else {
        if (!mReadStarted) {
            EmptyRoutine(); // The binary passes the path, which the merged routine ignores.
            mReadStarted = true;
        }
        if (mOffset != gAsyncBufSize) {
            _SeekToTell();
        }
        const int left = mSize - mTell;
        _ReadAsync(mBuffer, gAsyncBufSize < left ? gAsyncBufSize : left);
    }
    mOffset = 0;
    ASSERT(!mFail);
}

bool AsyncFile::Eof() {
    return mTell == mSize;
}

bool AsyncFile::Fail() {
    return mFail;
}

int AsyncFile::Size() {
    return mSize;
}

int AsyncFile::UncompressedSize() {
    return mUCSize;
}

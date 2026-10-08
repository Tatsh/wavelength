#include "os/asyncstream.h"

#include <cstring>

#include "os/mem.h"

namespace {

// The File::New() mode that opens a file for reading.
constexpr int kOpenRead = 1;
constexpr int kNoOpenFlags = 0;

// The byte count the constructor waits for, more than any file holds.
constexpr int kWholeFile = 999999999;

} // namespace

AsyncStream::AsyncStream(const char *pszFile, bool bLittleEndian)
    : BinStream(bLittleEndian), mFileName(pszFile) {
    mFail = 0;
    mPosition = 0;
    mBuffer = nullptr;
    mFile = nullptr;
    mSize = -1;
    mFile = File::New(pszFile, kOpenRead, kNoOpenFlags);
    mSize = mFile->Size();
    mBuffer = static_cast<char *>(PoolMemAlloc(mSize, "AsyncStream buf", 0));
    mFile->ReadAsync(mBuffer, mSize);
    Ready(kWholeFile);
}

AsyncStream::~AsyncStream() {
    if (mBuffer != nullptr) {
        PoolMemFree(mBuffer);
        mBuffer = nullptr;
    }
    delete mFile;
    mFile = nullptr;
}

bool AsyncStream::Ready(int nBytes) {
    if (mFile == nullptr) {
        return true;
    }
    int nRead;
    if (mFile->ReadDone(&nRead)) {
        delete mFile;
        mFile = nullptr;
        return true;
    }
    return nRead >= nBytes;
}

void AsyncStream::Read(void *pData, int nBytes) {
    if (mPosition + nBytes > mSize) {
        nBytes = mSize - mPosition;
        mFail = 1;
    }
    const int nEnd = mPosition + nBytes;
    while (!Ready(nEnd)) {
    }
    memcpy(pData, mBuffer + mPosition, nBytes);
    mPosition = nEnd;
}

void AsyncStream::Write(const void *pData, int nBytes) {
    if (mPosition + nBytes > mSize) {
        nBytes = mSize - mPosition;
        mFail = 1;
    }
    const int nEnd = mPosition + nBytes;
    while (!Ready(nEnd)) {
    }
    memcpy(mBuffer + mPosition, pData, nBytes);
    mPosition = nEnd;
}

void AsyncStream::Seek(int nOffset, SeekType eFrom) {
    int nPosition;
    switch (eFrom) {
    case kSeekBegin:
        nPosition = nOffset;
        break;
    case kSeekCurrent:
        nPosition = mPosition + nOffset;
        break;
    case kSeekEnd:
        nPosition = mSize + nOffset;
        break;
    default:
        return;
    }
    if (nPosition < 0 || nPosition > mSize) {
        mFail = 1;
        return;
    }
    mPosition = nPosition;
}

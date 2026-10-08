#include "os/asyncfile.h"

#include <cstring>

#include <sifdev.h>

#include "os/fileutil.h"
#include "os/mem.h"
#include "os/system.h"
#include "script/dataarray.h"

namespace {

// Descriptors the file system can have open.
constexpr int kMaxDescriptors = 16;

// The descriptor of a file that waits for a free descriptor. The next read opens it.
constexpr int kDescriptorDeferred = -12;

// The descriptor of a closed file.
constexpr int kNoDescriptor = -1;

// Alignment of the buffer.
constexpr int kBufferAlign = 64;

// Bytes of the size before compression at the end of a `.gz` file.
constexpr int kGzipSizeBytes = 4;

// Seek origins.
enum SeekOrigin { kSeekBegin = 0, kSeekCurrent = 1, kSeekEnd = 2 };

// NTSC-U/C: 0x003b2278
int gBufferSize = 8192;

// The file of each open descriptor.
// NTSC-U/C: 0x003b2280
AsyncFile *gDescriptorFiles[kMaxDescriptors];

// NTSC-U/C: 0x003b22c0
int gOpenDescriptors;

// Wait for the request of a descriptor to finish.
inline void WaitForDescriptor(int nFd) {
    int nBusy = 1;
    do {
        sceIoctl(nFd, SCE_FS_EXECUTING, &nBusy);
    } while (nBusy != 0);
}

} // namespace

void AsyncFile::Init() {
    SystemConfig()->FindArray("file", true)->FindInt("buf_size", &gBufferSize, false);
}

void AsyncFile::Terminate() {
}

AsyncFile::AsyncFile(const char *pszFile, int nMode)
    : mMode(0), mFd(kNoDescriptor), mTell(0), mBufferOffset(0), mReadStarted(0), mFail(0),
      mBuffer(nullptr), mReadDest(nullptr), mReadRemaining(0), mReadDone(0) {
    mBuffer = static_cast<char *>(PoolMemAlloc(gBufferSize, "AsyncFile buf", kBufferAlign));
    Open(pszFile, nMode);
}

void AsyncFile::Open(const char *pszFile, int nMode) {
    mMode = nMode;
    MakeDevicePath(mFilename, pszFile);
    mSize = OpenDescriptor();
    if (mFail != 0 || strcmp(FileGetExt(pszFile), "gz") != 0 || (mMode & kModeRead) == 0 ||
        mSize < kGzipSizeBytes) {
        mUncompressedSize = 0;
    } else {
        mTell = mSize - kGzipSizeBytes;
        SeekDescriptor();
        ReadDescriptor(&mUncompressedSize, kGzipSizeBytes);
        while (!DescriptorDone()) {
        }
        mTell = 0;
        SeekDescriptor();
    }
    if ((mMode & kModeRead) != 0) {
        mBufferOffset = gBufferSize;
        Flush();
    }
}

AsyncFile::~AsyncFile() {
    Close();
    PoolMemFree(mBuffer);
}

void AsyncFile::Close() {
    if ((mMode & kModeWrite) != 0) {
        Flush();
    }
    if (mFd >= 0) {
        CloseDescriptor();
    }
    mReadDone = 0;
    mFd = kNoDescriptor;
    mMode = 0;
    mTell = 0;
    mFail = 0;
    mBufferOffset = 0;
    mReadDest = nullptr;
    mReadRemaining = 0;
}

int AsyncFile::Read(void *pBuffer, int nBytes) {
    int nRead = nBytes;
    ReadAsync(pBuffer, nBytes);
    if (mFail != 0) {
        return 0;
    }
    while (!ReadDone(&nRead)) {
    }
    return nRead;
}

bool AsyncFile::ReadAsync(void *pBuffer, int nBytes) {
    if (mFail != 0) {
        return false;
    }
    if (mTell + nBytes > mSize) {
        nBytes = mSize - mTell;
    }
    mReadDest = static_cast<char *>(pBuffer);
    mReadRemaining = nBytes;
    mReadDone = 0;
    ReadDone(&nBytes);
    return true;
}

bool AsyncFile::ReadDone(int *pnBytes) {
    if (mFail != 0) {
        *pnBytes = 0;
        return true;
    }
    if (mReadRemaining == 0) {
        *pnBytes = mReadDone;
        return true;
    }
    if (!DescriptorDone()) {
        *pnBytes = mReadDone;
        return false;
    }
    if (mBufferOffset + mReadRemaining <= gBufferSize) {
        memcpy(mReadDest, mBuffer + mBufferOffset, mReadRemaining);
        mReadDone += mReadRemaining;
        mBufferOffset += mReadRemaining;
        mTell += mReadRemaining;
        mReadRemaining = 0;
        *pnBytes = mReadDone;
        return true;
    }
    const int nChunk = gBufferSize - mBufferOffset;
    memcpy(mReadDest, mBuffer + mBufferOffset, nChunk);
    mTell += nChunk;
    mReadDone += nChunk;
    mBufferOffset = gBufferSize;
    mReadRemaining -= nChunk;
    mReadDest += nChunk;
    Flush();
    return false;
}

int AsyncFile::Write(const void *pBuffer, int nBytes) {
    if (mFail != 0) {
        return 0;
    }
    const char *pIn = static_cast<const char *>(pBuffer);
    int nRemaining = nBytes;
    while (mBufferOffset + nRemaining > gBufferSize) {
        const int nChunk = gBufferSize - mBufferOffset;
        memcpy(mBuffer + mBufferOffset, pIn, nChunk);
        nRemaining -= nChunk;
        pIn += nChunk;
        mTell += nChunk;
        mBufferOffset = gBufferSize;
        Flush();
    }
    memcpy(mBuffer + mBufferOffset, pIn, nRemaining);
    mBufferOffset += nRemaining;
    mTell += nRemaining;
    if (mTell > mSize) {
        mSize = mTell;
    }
    return nBytes;
}

int AsyncFile::Seek(int nOffset, int nOrigin) {
    if (mFail != 0) {
        return mTell;
    }
    if ((mMode & kModeWrite) != 0) {
        Flush();
    }
    switch (nOrigin) {
    case kSeekBegin:
        mTell = nOffset;
        break;
    case kSeekCurrent:
        mTell += nOffset;
        break;
    case kSeekEnd:
        mTell = mSize + nOffset;
        break;
    default:
        break;
    }
    if (mTell < 0) {
        mTell = 0;
    } else if (mTell > mSize) {
        mTell = mSize;
    }
    SeekDescriptor();
    if ((mMode & kModeRead) != 0) {
        mBufferOffset = gBufferSize;
        Flush();
    }
    return mTell;
}

void AsyncFile::Flush() {
    if (mFail != 0) {
        return;
    }
    if ((mMode & kModeWrite) != 0) {
        WriteDescriptor(mBuffer, mBufferOffset);
        mBufferOffset = 0;
        return;
    }
    if (mReadStarted == 0) {
        FileNoteRead(mFilename.c_str());
        mReadStarted = 1;
    }
    if (mBufferOffset != gBufferSize) {
        SeekDescriptor();
    }
    const int nRemaining = mSize - mTell;
    ReadDescriptor(mBuffer, gBufferSize < nRemaining ? gBufferSize : nRemaining);
    mBufferOffset = 0;
}

void AsyncFile::FreeDescriptor() {
    int nIndex = 0;
    while (gOpenDescriptors == kMaxDescriptors) {
        AsyncFile *pFile = gDescriptorFiles[nIndex];
        if (pFile->DescriptorDone() && (pFile->mMode & kModeRead) != 0) {
            pFile->CloseDescriptor();
        }
        nIndex = (nIndex + 1) % kMaxDescriptors;
    }
}

int AsyncFile::OpenDescriptor() {
    int nSize;
    if (kImageFirstWord != 0) {
        FreeDescriptor();
        const int nFd = sceOpen(mFilename.c_str(), mMode);
        nSize = sceLseek(nFd, 0, SCE_SEEK_END);
        sceClose(nFd);
    } else {
        sce_stat stat;
        sceGetstat(mFilename.c_str(), &stat);
        nSize = static_cast<int>(stat.st_size);
    }
    mFd = sceOpen(mFilename.c_str(), mMode | SCE_NOWAIT);
    mFail = mFd < 0 && mFd != kDescriptorDeferred;
    if (mFd >= 0) {
        gDescriptorFiles[mFd] = this;
        ++gOpenDescriptors;
    }
    return nSize;
}

int AsyncFile::WriteDescriptor(const void *pBuffer, int nBytes) {
    if (sceWrite(mFd, pBuffer, nBytes) < 0) {
        mFail = 1;
    }
    WaitForDescriptor(mFd);
    return nBytes;
}

void AsyncFile::SeekDescriptor() {
    if (mFd < 0) {
        return;
    }
    if (sceLseek(mFd, mTell, SCE_SEEK_SET) < 0) {
        mFail = 1;
    }
    WaitForDescriptor(mFd);
}

void AsyncFile::ReadDescriptor(void *pBuffer, int nBytes) {
    if (mFd < 0) {
        FreeDescriptor();
        mFd = sceOpen(mFilename.c_str(), mMode | SCE_NOWAIT);
        ++gOpenDescriptors;
        gDescriptorFiles[mFd] = this;
        SeekDescriptor();
    }
    if (sceRead(mFd, pBuffer, nBytes) < 0) {
        mFail = 1;
    }
}

bool AsyncFile::DescriptorDone() {
    if (mFd < 0) {
        return true;
    }
    int nBusy = 1;
    sceIoctl(mFd, SCE_FS_EXECUTING, &nBusy);
    return nBusy == 0;
}

void AsyncFile::CloseDescriptor() {
    if (mFd < 0) {
        return;
    }
    if (sceClose(mFd) < 0) {
        mFail = 1;
    }
    mFd = kDescriptorDeferred;
    --gOpenDescriptors;
}

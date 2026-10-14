#include "os/filestreambuffer.h"

#include "os/debug.h"
#include "os/mem.h"

namespace {

constexpr int kBufferSize = 0xf000;

// The first read is half the buffer.
constexpr int kFirstReadSize = 30720;

constexpr int kOpenRead = 1;

// Origin of File::Seek() at the start of the file.
constexpr int kSeekStart = 0;

} // namespace

FileStreamBuffer::FileStreamBuffer(const char *pszPath, int nFlags) : StreamBuffer(kBufferSize) {
    mDelivered = 0;
    mFile = nullptr;
    mReadBuffer = nullptr;
    mReserved30 = 0;
    mReserved34 = 0;
    mFile = File::New(pszPath, kOpenRead, nFlags);
    if (mFile == nullptr) {
        DebugPrint("tried to load %s.\n", pszPath);
        return;
    }
    mReadBuffer = static_cast<char *>(PoolMemAlloc(kBufferSize, "FileStreamBuffer buf", 0));
    mFile->ReadAsync(mReadBuffer, kFirstReadSize);
}

FileStreamBuffer::~FileStreamBuffer() {
    delete mFile;
    mFile = nullptr;
    if (mReadBuffer != nullptr) {
        PoolMemFree(mReadBuffer);
        mReadBuffer = nullptr;
    }
}

void FileStreamBuffer::Poll() {
    if (mInputDone != 0) {
        return;
    }
    int nRead;
    if (!mFile->ReadDone(&nRead)) {
        return;
    }
    if (mDelivered == 0) {
        mDelivered = 1;
        const bool bEof = mFile->Eof();
        Add(mReadBuffer, nRead, bEof);
        if (bEof) {
            return;
        }
    }
    const int nRoom = EmptySpaceForFilling();
    if (nRoom != 0) {
        mDelivered = 0;
        mFile->ReadAsync(mReadBuffer, nRoom);
    }
}

void FileStreamBuffer::Rewind() {
    mFile->Seek(0, kSeekStart);
    mInputDone = 0;
}

bool FileStreamBuffer::IsClosed() const {
    return mFile == nullptr;
}

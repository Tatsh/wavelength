#include "utl/FileStream.h"

#include "os/Debug.h"

FileStream::FileStream(const char *file, FileType type, bool littleEndian, int flags)
    : BinStream(littleEndian) {
    const int mode =
        type != kRead ? FILE_OPEN_WRITE | FILE_OPEN_CREATE | FILE_OPEN_TRUNCATE : FILE_OPEN_READ;
    mFile = NewFile(file, mode, flags);
    mFail = mFile == NULL;
}

FileStream::~FileStream() {
    delete mFile;
}

void FileStream::Read(void *data, int bytes) {
    ASSERT(!mFail);
    if (mFile->Read(data, bytes) != bytes) {
        mFail = true;
    }
}

void FileStream::Write(const void *data, int bytes) {
    ASSERT(!mFail);
    if (mFile->Write(data, bytes) != bytes) {
        mFail = true;
    }
}

void FileStream::Flush() {
    ASSERT(!mFail);
    mFile->Flush();
}

void FileStream::Seek(int offset, SeekType type) {
    // The file's origins, indexed by SeekType.
    const int origins[] = { 0, 1, 2 };
    ASSERT(!mFail);
    if (mFile->Seek(offset, origins[type]) < 0) {
        mFail = true;
    }
}

int FileStream::Tell() {
    ASSERT(!mFail);
    return mFile->Tell();
}

bool FileStream::Eof() {
    ASSERT(!mFail);
    return mFile->Eof();
}

bool FileStream::Fail() {
    return mFail;
}

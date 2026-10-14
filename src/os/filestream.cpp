#include "os/filestream.h"

namespace {

// File::New() mode of a writer, which creates or truncates the file.
constexpr int kOpenWrite = 0x602;

} // namespace

FileStream::FileStream(const char *pszPath, bool bWrite, bool bLittleEndian, int nFlags)
    : BinStream(bLittleEndian) {
    mFile = File::New(pszPath, bWrite ? kOpenWrite : File::kModeRead, nFlags);
    mFail = mFile == nullptr;
}

FileStream::~FileStream() {
    delete mFile;
}

void FileStream::Read(void *pData, int nBytes) {
    if (mFile->Read(pData, nBytes) != nBytes) {
        mFail = 1;
    }
}

void FileStream::Write(const void *pData, int nBytes) {
    if (mFile->Write(pData, nBytes) != nBytes) {
        mFail = 1;
    }
}

void FileStream::Flush() {
    mFile->Flush();
}

void FileStream::Seek(int nOffset, SeekType eFrom) {
    // The File::Seek() origin of each SeekType.
    const int anOrigins[] = {0, 1, 2};
    if (mFile->Seek(nOffset, anOrigins[eFrom]) < 0) {
        mFail = 1;
    }
}

int FileStream::Tell() {
    return mFile->Tell();
}

bool FileStream::Eof() {
    return mFile->Eof();
}

bool FileStream::Fail() {
    return mFail != 0;
}

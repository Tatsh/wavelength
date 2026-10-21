#include "os/filestream.h"

namespace {

// File::New() mode of a writer, which creates or truncates the file.
constexpr int kOpenWrite = 0x602;

} // namespace

FileStream::FileStream(const char *pszPath, bool bWrite, bool bLittleEndian, int nFlags)
    : BinStream(bLittleEndian) {
    mFile.reset(File::New(pszPath, bWrite ? kOpenWrite : File::kModeRead, nFlags));
    mFail = mFile == nullptr;
}

FileStream::~FileStream() = default;

#ifdef VIDEO_STANDARD_PAL
void FileStream::ReadImpl(void *pData, int nBytes) {
#else
void FileStream::Read(void *pData, int nBytes) {
#endif
    if (mFile->Read(pData, nBytes) != nBytes) {
        mFail = 1;
    }
}

#ifdef VIDEO_STANDARD_PAL
void FileStream::WriteImpl(const void *pData, int nBytes) {
#else
void FileStream::Write(const void *pData, int nBytes) {
#endif
    if (mFile->Write(pData, nBytes) != nBytes) {
        mFail = 1;
    }
}

void FileStream::Flush() {
    mFile->Flush();
}

#ifdef VIDEO_STANDARD_PAL
void FileStream::SeekImpl(int nOffset, SeekType eFrom) {
#else
void FileStream::Seek(int nOffset, SeekType eFrom) {
#endif
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

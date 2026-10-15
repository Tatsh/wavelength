#include "os/memstream.h"

#include <string.h>

namespace {

// The bytes the buffer has room for at construction, and the room each full Write() adds.
constexpr int kMemStreamGrowth = 4096;

} // namespace

MemStream::MemStream(bool bLittleEndian) : BinStream(bLittleEndian) {
    mBuffer.reserve(kMemStreamGrowth);
    mFail = 0;
    mTell = 0;
}

#ifdef VIDEO_STANDARD_PAL
void MemStream::ReadImpl(void *pData, int nBytes) {
#else
void MemStream::Read(void *pData, int nBytes) {
#endif
    const unsigned int nSize = mBuffer.size();
    if (nSize < static_cast<unsigned int>(mTell + nBytes)) {
        nBytes = nSize - mTell;
        mFail = 1;
    }
    memcpy(pData, &mBuffer[mTell], nBytes);
    mTell += nBytes;
}

#ifdef VIDEO_STANDARD_PAL
void MemStream::WriteImpl(const void *pData, int nBytes) {
#else
void MemStream::Write(const void *pData, int nBytes) {
#endif
    const unsigned int nCapacity = mBuffer.capacity();
    if (nCapacity < static_cast<unsigned int>(mTell + nBytes)) {
        mBuffer.reserve(nCapacity + kMemStreamGrowth);
    }
    if (mBuffer.size() < static_cast<unsigned int>(mTell + nBytes)) {
        mBuffer.resize(mTell + nBytes, 0);
    }
    memcpy(&mBuffer[mTell], pData, nBytes);
    mTell += nBytes;
}

void MemStream::Flush() {
}

#ifdef VIDEO_STANDARD_PAL
void MemStream::SeekImpl(int nOffset, SeekType eFrom) {
#else
void MemStream::Seek(int nOffset, SeekType eFrom) {
#endif
    switch (eFrom) {
    case kSeekBegin:
        break;
    case kSeekCurrent:
        nOffset += mTell;
        break;
    case kSeekEnd:
        nOffset += mBuffer.size();
        break;
    default:
        return;
    }
    if (nOffset < 0 || mBuffer.size() < static_cast<unsigned int>(nOffset)) {
        mFail = 1;
        return;
    }
    mTell = nOffset;
}

int MemStream::Tell() {
    return mTell;
}

bool MemStream::Eof() {
    return static_cast<unsigned int>(mTell) == mBuffer.size();
}

bool MemStream::Fail() {
    return mFail != 0;
}

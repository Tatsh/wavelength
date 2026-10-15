#include "os/bufstream.h"

#include <cstring>

BufStream::BufStream(char *pBuffer, int nSize, bool bLittleEndian) : BinStream(bLittleEndian) {
    mFail = pBuffer == nullptr;
    mSize = nSize;
    mBuffer = pBuffer;
    mPosition = 0;
}

#ifdef VIDEO_STANDARD_PAL
void BufStream::ReadImpl(void *pData, int nBytes) {
#else
void BufStream::Read(void *pData, int nBytes) {
#endif
    if (mPosition + nBytes > mSize) {
        nBytes = mSize - mPosition;
        mFail = 1;
    }
    memcpy(pData, mBuffer + mPosition, nBytes);
    mPosition += nBytes;
}

#ifdef VIDEO_STANDARD_PAL
void BufStream::WriteImpl(const void *pData, int nBytes) {
#else
void BufStream::Write(const void *pData, int nBytes) {
#endif
    if (mPosition + nBytes > mSize) {
        nBytes = mSize - mPosition;
        mFail = 1;
    }
    memcpy(mBuffer + mPosition, pData, nBytes);
    mPosition += nBytes;
}

#ifdef VIDEO_STANDARD_PAL
void BufStream::SeekImpl(int nOffset, SeekType eFrom) {
#else
void BufStream::Seek(int nOffset, SeekType eFrom) {
#endif
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

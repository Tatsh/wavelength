#include "utl/BufStream.h"

#include <string.h>

BufStream::BufStream(char *buffer, int size, bool littleEndian)
    : BinStream(littleEndian), mBuffer(buffer), mFail(buffer == NULL), mTell(0), mSize(size) {
}

BufStream::~BufStream() {
}

void BufStream::Read(void *data, int bytes) {
    if (mTell + bytes > mSize) {
        bytes = mSize - mTell;
        mFail = true;
    }
    memcpy(data, mBuffer + mTell, bytes);
    mTell += bytes;
}

void BufStream::Write(const void *data, int bytes) {
    if (mTell + bytes > mSize) {
        bytes = mSize - mTell;
        mFail = true;
    }
    memcpy(mBuffer + mTell, data, bytes);
    mTell += bytes;
}

void BufStream::Seek(int offset, SeekType type) {
    int position;
    switch (type) {
    case kSeekBegin:
        position = offset;
        break;
    case kSeekCurrent:
        position = mTell + offset;
        break;
    case kSeekEnd:
        position = mSize + offset;
        break;
    default:
        return;
    }
    if (position < 0 || position > mSize) {
        mFail = true;
        return;
    }
    mTell = position;
}

int BufStream::Tell() {
    return mTell;
}

bool BufStream::Eof() {
    return mTell == mSize;
}

bool BufStream::Fail() {
    return mFail;
}

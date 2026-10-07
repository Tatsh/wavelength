#include "softfx_s/ringbuffer.h"

#include <sysclib.h>

namespace {

constexpr unsigned int kDefaultReserve = 16;

// NTSC-U/C: 0x000065a0
bool g_bRingBufferWriting;

} // namespace

RingBuffer::RingBuffer(unsigned int size)
    : mSize(size), mBuffer(nullptr), mRead(nullptr), mEnd(nullptr), mPendingRead(nullptr),
      mEndOfStream(false), mReserve(kDefaultReserve) {
    mBuffer = new char[size / sizeof(int) * sizeof(int)];
    memset(mBuffer, 0, mSize);
    mEnd = mBuffer + mSize;
    mWrite = mBuffer;
    mRead = mBuffer;
}

RingBuffer::~RingBuffer() {
    if (mBuffer != nullptr) {
        delete[] mBuffer;
    }
    mBuffer = nullptr;
}

bool RingBuffer::IsFinished() const {
    if (mWrite >= mEnd) {
        return true;
    }
    return mRead == mWrite && mEndOfStream;
}

unsigned int RingBuffer::ContiguousFree() const {
    if (mWrite < mRead) {
        return mRead - mWrite - mReserve;
    }
    const unsigned int free = mEnd - mWrite;
    const unsigned int readOffset = mRead - mBuffer;
    if (readOffset < mReserve) {
        return free - (mReserve - readOffset);
    }
    return free;
}

bool RingBuffer::IsFull() const {
    return ContiguousFree() == 0;
}

bool RingBuffer::IsEmpty() const {
    return mRead == mWrite;
}

void *RingBuffer::Read(unsigned int maxSize, unsigned int *size) {
    Notify();
    if (IsFinished() || IsEmpty()) {
        return nullptr;
    }
    const unsigned int available = mRead < mWrite ? mWrite - mRead : mEnd - mRead;
    if (available < maxSize) {
        maxSize = available;
    }
    *size = maxSize;
    mPendingRead = mRead + maxSize;
    return mRead;
}

void RingBuffer::Release(void **block) {
    if (*block == nullptr) {
        return;
    }
    mRead = mPendingRead;
    if (mPendingRead == mEnd) {
        mRead = mBuffer;
    }
    mPendingRead = nullptr;
    Notify();
    *block = nullptr;
}

void RingBuffer::Write(const void *data, unsigned int size, bool endOfStream) {
    g_bRingBufferWriting = true;
    (void)FreeSpace(); // Yes, the binary discards both reports.
    (void)FreeSpace();
    mEndOfStream = endOfStream;
    if (data == nullptr || size == 0) {
        return; // The binary leaves the writing flag set here.
    }
    if (mWrite < mRead || static_cast<unsigned int>(mEnd - mWrite) >= size) {
        memcpy(mWrite, data, size);
        mWrite += size;
        if (mWrite == mEnd) {
            mWrite = mBuffer;
        }
    } else {
        const unsigned int first = mEnd - mWrite;
        const unsigned int rest = size - first;
        memcpy(mWrite, data, first);
        memcpy(mBuffer, static_cast<const char *>(data) + first, rest);
        mWrite = mBuffer + rest;
    }
    g_bRingBufferWriting = false;
}

unsigned int RingBuffer::FreeSpace() const {
    bool endOfStream;
    return mSize - (Used(&endOfStream) + mReserve);
}

unsigned int RingBuffer::Used(bool *endOfStream) const {
    unsigned int used = mWrite - mRead;
    if (mWrite < mRead) {
        used += mSize;
    }
    *endOfStream = mEndOfStream;
    return used;
}

void RingBuffer::Reset() {
    mWrite = mBuffer;
    mRead = mBuffer;
}

void RingBuffer::ClearEndOfStream() {
    mEndOfStream = false;
}

bool RingBuffer::EndOfStream() const {
    return mEndOfStream;
}

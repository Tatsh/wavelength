#include "libnetb/asyncblock.h"

namespace {

constexpr int kResultNullArgument = 2;

} // namespace

int AsyncBlock::InitRing(unsigned int size) {
    if (size < 2 * kHeaderSize) {
        return kResultNullArgument;
    }
    mCapacity = size - kHeaderSize;
    mLength = size - kHeaderSize;
    mPadding = 0;
    mUsed = 0;
    mValid = 0;
    return 0;
}

int AsyncBlock::MeasureFree(unsigned int size, unsigned int *free) {
    if (free == nullptr) {
        return kResultNullArgument;
    }
    *free = 0;
    unsigned int total = 0;
    AsyncBlock *block = this;
    while (size >= kHeaderSize) {
        if (block->mUsed != 0 || block->mValid != 0) {
            break;
        }
        const unsigned int step = block->mCapacity + kHeaderSize;
        total += step;
        size -= step;
        block = At(block->mData, block->mCapacity);
    }
    *free = total;
    return 0;
}

AsyncBlock *AsyncBlock::At(unsigned char *ring, unsigned int offset) {
    return reinterpret_cast<AsyncBlock *>(&ring[offset]); // Blocks lie end to end in bytes.
}

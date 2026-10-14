#include "os/bitstream.h"

#include <string.h>

namespace {

constexpr unsigned int kByteShift = 3;
constexpr unsigned int kBitMask = 7;

} // namespace

BitStream::BitStream(void *pBuffer, int nSize) {
    mBuffer = static_cast<unsigned char *>(pBuffer);
    mSize = nSize;
    mBitPos = 0;
}

void BitStream::PackBool(bool bValue) {
    Write(bValue, 1);
}

void BitStream::PackSigned(int nValue, int nBits) {
    Write(nValue, nBits);
}

void BitStream::Pack(unsigned int nValue, int nBits) {
    Write(nValue, nBits);
}

void BitStream::Write(unsigned int nValue, unsigned int nBits) {
    if (mBitPos == 0) {
        memset(mBuffer, 0, mSize);
    }
    for (unsigned int i = 0; i < nBits; ++i) {
        mBuffer[mBitPos >> kByteShift] |= ((nValue >> i) & 1) << (mBitPos & kBitMask);
        ++mBitPos;
    }
}

bool BitStream::UnpackBool() {
    return Unpack(1) != 0;
}

int BitStream::UnpackSigned(int nBits) {
    const int nHalf = 1 << (nBits - 1);
    int nValue = Unpack(nBits);
    if (nValue >= nHalf) {
        nValue -= nHalf * 2;
    }
    return nValue;
}

unsigned int BitStream::Unpack(int nBits) {
    unsigned int nValue = 0;
    for (unsigned int i = 0; i < static_cast<unsigned int>(nBits); ++i) {
        const unsigned int nBit = (mBuffer[mBitPos >> kByteShift] >> (mBitPos & kBitMask)) & 1;
        ++mBitPos;
        nValue |= nBit << i;
    }
    return nValue;
}

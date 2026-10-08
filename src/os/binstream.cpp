#include "os/binstream.h"

#include <cstring>

namespace {

// Sizes of the values ReadEndian() and WriteEndian() reorder.
enum EndianSize {
    kEndianSize16 = 2,
    kEndianSize32 = 4,
    kEndianSize64 = 8,
    kEndianSize128 = 16,
};

// Halves of a 128-bit value.
enum Half { kHalfLow = 0, kHalfHigh = 1, kHalfCount = 2 };

} // namespace

void BinStream::ReadEndian(void *pData, int nBytes) {
    Read(pData, nBytes);
    if (mLittleEndian) {
        return;
    }
    switch (nBytes) {
    case kEndianSize16: {
        auto *pValue = static_cast<unsigned short *>(pData);
        *pValue = __builtin_bswap16(*pValue);
        break;
    }
    case kEndianSize32: {
        auto *pValue = static_cast<unsigned int *>(pData);
        *pValue = __builtin_bswap32(*pValue);
        break;
    }
    case kEndianSize64: {
        auto *pValue = static_cast<unsigned long long *>(pData);
        *pValue = __builtin_bswap64(*pValue);
        break;
    }
    case kEndianSize128: {
        // Yes, the high half receives the reversed low half and the low half is left as read.
        auto *pValue = static_cast<unsigned long long *>(pData);
        pValue[kHalfHigh] = __builtin_bswap64(pValue[kHalfLow]);
        pValue[kHalfLow] = __builtin_bswap64(pValue[kHalfHigh]);
        break;
    }
    default:
        break;
    }
}

void BinStream::WriteEndian(const void *pData, int nBytes) {
    if (mLittleEndian) {
        Write(pData, nBytes);
        return;
    }
    unsigned long long anSwapped[kHalfCount];
    switch (nBytes) {
    case kEndianSize16: {
        const unsigned short nValue = *static_cast<const unsigned short *>(pData);
        const unsigned short nSwapped = __builtin_bswap16(nValue);
        memcpy(anSwapped, &nSwapped, sizeof(nSwapped));
        break;
    }
    case kEndianSize32: {
        const unsigned int nValue = *static_cast<const unsigned int *>(pData);
        const unsigned int nSwapped = __builtin_bswap32(nValue);
        memcpy(anSwapped, &nSwapped, sizeof(nSwapped));
        break;
    }
    case kEndianSize64:
        anSwapped[kHalfLow] = __builtin_bswap64(*static_cast<const unsigned long long *>(pData));
        break;
    case kEndianSize128: {
        const auto *pValue = static_cast<const unsigned long long *>(pData);
        anSwapped[kHalfLow] = __builtin_bswap64(pValue[kHalfHigh]);
        anSwapped[kHalfHigh] = __builtin_bswap64(pValue[kHalfLow]);
        break;
    }
    default:
        break;
    }
    Write(anSwapped, nBytes);
}

BinStream &BinStream::WriteString(const char *pszText) {
    const int nLength = static_cast<int>(strlen(pszText));
    WriteEndian(&nLength, sizeof(nLength));
    Write(pszText, nLength);
    return *this;
}

void BinStream::ReadString(String &text) {
    int nLength;
    ReadEndian(&nLength, sizeof(nLength));
    text.Resize(nLength);
    Read(text.mBuffer, nLength);
}

void BinStream::ReadString(char *pszBuffer) {
    int nLength;
    ReadEndian(&nLength, sizeof(nLength));
    Read(pszBuffer, nLength);
    pszBuffer[nLength] = '\0';
}

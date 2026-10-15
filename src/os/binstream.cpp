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

#ifdef VIDEO_STANDARD_PAL
// Bytes Write() encrypts into its buffer for each WriteImpl() call.
constexpr int kCryptChunkSize = 256;
#endif

} // namespace

#ifdef VIDEO_STANDARD_PAL
BinStream::~BinStream() {
    delete mCrypto;
}

void BinStream::EnableReadEncryption() {
    int nSeed;
    ReadEndian(&nSeed, sizeof(nSeed));
    mCrypto = new Rand(nSeed);
}

void BinStream::DisableEncryption() {
    delete mCrypto;
    mCrypto = nullptr;
}

void BinStream::Read(void *pData, int nBytes) {
    ReadImpl(pData, nBytes);
    if (mCrypto == nullptr) {
        return;
    }
    auto *pByte = static_cast<unsigned char *>(pData);
    for (unsigned char *pEnd = pByte + nBytes; pByte < pEnd; ++pByte) {
        *pByte ^= static_cast<unsigned char>(mCrypto->Int());
    }
}

void BinStream::Write(const void *pData, int nBytes) {
    if (mCrypto == nullptr) {
        WriteImpl(pData, nBytes);
        return;
    }
    const auto *pSource = static_cast<const unsigned char *>(pData);
    unsigned char abChunk[kCryptChunkSize];
    for (; nBytes > 0; nBytes -= kCryptChunkSize) {
        const int nChunk = nBytes < kCryptChunkSize ? nBytes : kCryptChunkSize;
        for (int i = 0; i < nChunk; ++i) {
            abChunk[i] = pSource[i] ^ static_cast<unsigned char>(mCrypto->Int());
        }
        WriteImpl(abChunk, nChunk);
        pSource += kCryptChunkSize;
    }
}

void BinStream::Seek(int nOffset, SeekType eFrom) {
    SeekImpl(nOffset, eFrom);
}
#endif

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

void BinStream::ReadString(char *pszBuffer, int /* nSize */) {
    int nLength;
    ReadEndian(&nLength, sizeof(nLength));
    Read(pszBuffer, nLength);
    pszBuffer[nLength] = '\0';
}

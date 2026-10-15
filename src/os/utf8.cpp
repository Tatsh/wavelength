#include "os/utf8.h"

#ifdef VIDEO_STANDARD_PAL

#include <cstring>

namespace {

constexpr int kAsciiBytes = 1;
constexpr int kLatinBytes = 2;
constexpr int kThreeByteSequence = 3;
constexpr int kFourByteSequence = 4;
constexpr int kTerminatorBytes = 1;

constexpr int kLeadShift = 6;
constexpr unsigned int kTwoByteLead = 0xc0;
constexpr unsigned int kContinuationMarker = 0x80;
constexpr unsigned int kPayloadMask = 0x3f;

constexpr unsigned int kLeadNibbleMask = 0xf0;
constexpr unsigned int kThreeByteLead = 0xe0;
constexpr unsigned int kFourByteLead = 0xf0;

constexpr int kAsciiShift = 7;
constexpr int kTwoByteShift = 5;
constexpr unsigned int kTwoByteTag = 6;
constexpr int kThreeByteShift = 4;
constexpr unsigned int kThreeByteTag = 14;

} // namespace

int Utf8Size(const char *pszText) {
    int nSize = 0;
    for (unsigned int i = 0; i < std::strlen(pszText); ++i) {
        nSize += static_cast<signed char>(pszText[i]) < 0 ? kLatinBytes : kAsciiBytes;
    }
    return nSize + kTerminatorBytes;
}

void Utf8Encode(char *pszOut, const char *pszText, int nSize) {
    (void)nSize;
    int nOut = 0;
    for (unsigned int i = 0; i < std::strlen(pszText); ++i) {
        const auto ch = static_cast<unsigned char>(pszText[i]);
        if (static_cast<signed char>(pszText[i]) >= 0) {
            pszOut[nOut] = static_cast<char>(ch);
        } else {
            pszOut[nOut++] = static_cast<char>((ch >> kLeadShift) | kTwoByteLead);
            pszOut[nOut] = static_cast<char>((ch & kPayloadMask) | kContinuationMarker);
        }
        ++nOut;
    }
    pszOut[nOut] = '\0';
}

int Utf8DecodedSize(const char *pszUtf8) {
    unsigned int nIn = 0;
    int nCount = 0;
    while (nIn < std::strlen(pszUtf8)) {
        const unsigned int nLead = static_cast<unsigned char>(pszUtf8[nIn]) & kLeadNibbleMask;
        if (nLead == kFourByteLead) {
            nIn += kFourByteSequence;
        } else if (nLead == kThreeByteLead) {
            nIn += kThreeByteSequence;
        } else if (nLead == kTwoByteLead) {
            nIn += kLatinBytes;
        } else {
            nIn += kAsciiBytes;
        }
        ++nCount;
    }
    return nCount + kTerminatorBytes;
}

void Utf8Decode(char *pszOut, const char *pszUtf8, int nSize) {
    (void)nSize;
    unsigned int nIn = 0;
    int nCount = 0;
    char *pszWrite = pszOut;
    while (nIn < std::strlen(pszUtf8)) {
        const auto ch = static_cast<unsigned char>(pszUtf8[nIn]);
        if ((ch >> kAsciiShift) == 0) {
            *pszWrite = static_cast<char>(ch);
            nIn += kAsciiBytes;
        } else if ((ch >> kTwoByteShift) == kTwoByteTag) {
            const auto next = static_cast<unsigned char>(pszUtf8[nIn + 1]);
            *pszWrite = static_cast<char>((ch << kLeadShift) | (next & kPayloadMask));
            nIn += kLatinBytes;
        } else if ((ch >> kThreeByteShift) == kThreeByteTag) {
            nIn += kThreeByteSequence;
        } else {
            nIn += kFourByteSequence;
        }
        ++pszWrite;
        ++nCount;
    }
    pszOut[nCount] = '\0';
}

#endif

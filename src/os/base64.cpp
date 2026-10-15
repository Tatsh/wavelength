#include "os/base64.h"

namespace {

constexpr int kGroupBytes = 3;
constexpr int kGroupChars = 4;

// NTSC-U/C: 0x0041fef8, PAL: 0x004b7c00
constexpr char kAlphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz. ";

// The value of each character of kAlphabet, indexed by the character.
// NTSC-U/C: 0x0041ff40, PAL: 0x004b7c48
constexpr unsigned char kDecode[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  63, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  62, 0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,  0,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 0,  0,  0,  0,  0,  0,  36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0};

// Three bytes seen as four 6-bit digits, lowest first.
union Group {
    unsigned char mBytes[kGroupBytes];
    struct {
        unsigned int mFirst : 6;
        unsigned int mSecond : 6;
        unsigned int mThird : 6;
        unsigned int mFourth : 6;
    } mDigits;
};

inline void DecodeGroup(Group &group, const char *pszIn) {
    group.mDigits.mFirst = kDecode[static_cast<unsigned char>(pszIn[0])];
    group.mDigits.mSecond = kDecode[static_cast<unsigned char>(pszIn[1])];
    group.mDigits.mThird = kDecode[static_cast<unsigned char>(pszIn[2])];
    group.mDigits.mFourth = kDecode[static_cast<unsigned char>(pszIn[3])];
}

} // namespace

void Base64Encode(char *pszOut, int *pnOutLength, const unsigned char *pIn, int nInLength) {
    const unsigned char *pEnd = &pIn[nInLength];
    *pszOut++ = kAlphabet[nInLength % kGroupBytes];
    *pnOutLength = 1;
    while (pIn < pEnd) {
        Group group;
        group.mBytes[0] = *pIn++;
        group.mBytes[1] = *pIn++;
        group.mBytes[2] = *pIn++;
        *pszOut++ = kAlphabet[group.mDigits.mFirst];
        *pszOut++ = kAlphabet[group.mDigits.mSecond];
        *pszOut++ = kAlphabet[group.mDigits.mThird];
        *pszOut++ = kAlphabet[group.mDigits.mFourth];
        *pnOutLength += kGroupChars;
    }
}

void Base64Decode(unsigned char *pOut, int *pnOutLength, const char *pszIn, int nInLength) {
    *pnOutLength = 0;
    const char *pEnd = &pszIn[nInLength];
    int nPartial = kDecode[static_cast<unsigned char>(*pszIn++)];
    if (nPartial != 0) {
        pEnd = &pEnd[-kGroupChars];
    }
    Group group;
    while (pszIn < pEnd) {
        DecodeGroup(group, pszIn);
        pszIn = &pszIn[kGroupChars];
        *pOut++ = group.mBytes[0];
        *pOut++ = group.mBytes[1];
        *pOut++ = group.mBytes[2];
        *pnOutLength += kGroupBytes;
    }
    if (nPartial != 0) {
        DecodeGroup(group, pszIn);
        *pOut++ = group.mBytes[0];
        ++*pnOutLength;
        if (--nPartial != 0) {
            *pOut = group.mBytes[1];
            ++*pnOutLength;
        }
    }
}

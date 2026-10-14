#include "os/string.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "os/binstream.h"
#include "os/mem.h"

namespace {

constexpr int kFormatBufferSize = 0x400;
constexpr int kScratchSize = 256;

// NTSC-U/C: 0x00515a18
char g_szFormatBuffer[kFormatBufferSize] = {};

// The text of every empty string, which is never written.
// NTSC-U/C: 0x0041fc88
char g_szStringEmptyText[4] = {};

// NTSC-U/C: 0x00515e18
char g_szStreamCharScratch[kScratchSize];

} // namespace

char *g_szStringEmptyBuffer = g_szStringEmptyText;

// NTSC-U/C: 0x003b24c8
static int g_nStringReadsText = 0;

// NTSC-U/C: 0x003b24cc
static const char *g_pszStringTag = "StringBuf";

const char *FormatString(const char *pszFormat, ...) {
    va_list args;
    va_start(args, pszFormat);
    vsprintf(g_szFormatBuffer, pszFormat, args);
    va_end(args);
    return g_szFormatBuffer;
}

String::String(const char *pszText) {
    if (pszText == nullptr) {
        mLength = 0;
        mBuffer = g_szStringEmptyBuffer;
        return;
    }
    const int nLength = static_cast<int>(strlen(pszText));
    mLength = nLength;
    mCapacity = nLength + 1;
    mBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    strcpy(mBuffer, pszText);
}

String::String(const String &other) : PrnStream() {
    if (other.mBuffer == g_szStringEmptyBuffer) {
        mBuffer = other.mBuffer;
        mLength = 0;
        return;
    }
    mLength = other.mLength;
    mCapacity = other.mLength + 1;
    mBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    strcpy(mBuffer, other.mBuffer);
}

String::~String() {
    if (mBuffer != g_szStringEmptyBuffer) {
        MemOrPoolFree(mCapacity, mBuffer);
    }
}

void String::Print(const char *pszText) {
    *this += pszText;
}

String String::operator+(const char *pszText) const {
    String result(*this);
    result += pszText;
    return result;
}

String String::operator+(char ch) const {
    String result(*this);
    result += ch;
    return result;
}

String String::operator+(const String &other) const {
    String result(*this);
    result += other;
    return result;
}

String &String::operator+=(const String &other) {
    if (other.mBuffer == g_szStringEmptyBuffer) {
        return *this;
    }
    const int nLength = mLength + other.mLength;
    const int nOldCapacity = mCapacity;
    mCapacity = nLength + 1;
    char *pBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    if (mBuffer != g_szStringEmptyBuffer) {
        strcpy(pBuffer, mBuffer);
        MemOrPoolFree(nOldCapacity, mBuffer);
    }
    strcpy(&pBuffer[mLength], other.mBuffer);
    mBuffer = pBuffer;
    mLength = nLength;
    return *this;
}

String &String::operator+=(const char *pszText) {
    if (pszText == nullptr) {
        return *this;
    }
    const int nLength = mLength + static_cast<int>(strlen(pszText));
    const int nOldCapacity = mCapacity;
    mCapacity = nLength + 1;
    char *pBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    if (mBuffer != g_szStringEmptyBuffer) {
        strcpy(pBuffer, mBuffer);
        MemOrPoolFree(nOldCapacity, mBuffer);
    }
    strcpy(&pBuffer[mLength], pszText);
    mBuffer = pBuffer;
    mLength = nLength;
    return *this;
}

String &String::operator+=(char ch) {
    const int nOldCapacity = mCapacity;
    const int nOldLength = mLength;
    mLength = nOldLength + 1;
    mCapacity = nOldLength + 2;
    char *pBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    if (mBuffer != g_szStringEmptyBuffer) {
        strcpy(pBuffer, mBuffer);
        MemOrPoolFree(nOldCapacity, mBuffer);
    }
    pBuffer[mLength - 1] = ch;
    pBuffer[mLength] = '\0';
    mBuffer = pBuffer;
    return *this;
}

String &String::operator=(const char *pszText) {
    if (pszText == mBuffer) {
        return *this;
    }
    if (mBuffer != g_szStringEmptyBuffer) {
        MemOrPoolFree(mCapacity, mBuffer);
    }
    if (pszText == nullptr) {
        mLength = 0;
        mBuffer = g_szStringEmptyBuffer;
        return *this;
    }
    const int nLength = static_cast<int>(strlen(pszText));
    mLength = nLength;
    mCapacity = nLength + 1;
    mBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    strcpy(mBuffer, pszText);
    return *this;
}

String &String::operator=(const String &other) {
    if (&other == this) {
        return *this;
    }
    if (mBuffer != g_szStringEmptyBuffer) {
        MemOrPoolFree(mCapacity, mBuffer);
    }
    if (other.mBuffer == g_szStringEmptyBuffer) {
        mBuffer = other.mBuffer;
        mLength = 0;
        return *this;
    }
    // The length is measured again rather than copied.
    const int nLength = static_cast<int>(strlen(other.mBuffer));
    mLength = nLength;
    mCapacity = nLength + 1;
    mBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    strcpy(mBuffer, other.mBuffer);
    return *this;
}

char String::operator[](int nIndex) const {
    return mBuffer[nIndex];
}

bool String::operator!=(const char *pszText) const {
    if (pszText == nullptr) {
        return true;
    }
    return strcmp(pszText, mBuffer) != 0;
}

bool String::operator!=(const String &other) const {
    return strcmp(other.mBuffer, mBuffer) != 0;
}

bool String::operator==(const char *pszText) const {
    if (pszText == nullptr) {
        return false;
    }
    return strcmp(pszText, mBuffer) == 0;
}

bool String::operator==(const String &other) const {
    return strcmp(other.mBuffer, mBuffer) == 0;
}

bool String::operator<(const String &other) const {
    return strcmp(mBuffer, other.mBuffer) < 0;
}

void String::Resize(int nLength) {
    if (mBuffer != g_szStringEmptyBuffer) {
        MemOrPoolFree(mCapacity, mBuffer);
    }
    mLength = nLength;
    mCapacity = nLength + 1;
    mBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    memset(mBuffer, 0, mLength + 1);
}

int String::Find(char ch) const {
    for (const char *p = mBuffer; *p != '\0'; ++p) {
        if (*p == ch) {
            return p - mBuffer;
        }
    }
    return npos;
}

int String::Find(char ch, int nPos) const {
    if (!(static_cast<unsigned int>(nPos) < static_cast<unsigned int>(mLength))) {
        return npos;
    }
    for (const char *p = &mBuffer[nPos]; *p != '\0'; ++p) {
        if (*p == ch) {
            return p - mBuffer;
        }
    }
    return npos;
}

int String::Find(const char *pszText) const {
    const char *pFound = strstr(mBuffer, pszText);
    if (pFound == nullptr) {
        return npos;
    }
    return pFound - mBuffer;
}

int String::RFind(char ch) const {
    const char *pFound = strrchr(mBuffer, ch);
    if (pFound == nullptr) {
        return npos;
    }
    return pFound - mBuffer;
}

int String::Compare(int nPos, int nCount, const char *pszText) const {
    if (pszText == nullptr) {
        return -1;
    }
    return strncmp(&mBuffer[nPos], pszText, nCount);
}

String String::Substring(int nPos) const {
    return String(&mBuffer[nPos]);
}

String String::Substring(int nPos, int nCount) const {
    if (!(static_cast<unsigned int>(nPos + nCount) < static_cast<unsigned int>(mLength))) {
        return Substring(nPos);
    }
    String result;
    result.mCapacity = nCount + 1;
    result.mLength = nCount;
    result.mBuffer = static_cast<char *>(MemOrPoolAlloc(result.mCapacity, g_pszStringTag));
    strncpy(result.mBuffer, &mBuffer[nPos], nCount);
    result.mBuffer[nCount] = '\0';
    return result;
}

String &String::Replace(int nPos, int nCount, const String &text) {
    int nRemove = mLength - nPos;
    if (!(static_cast<unsigned int>(mLength) < static_cast<unsigned int>(nPos + nCount))) {
        nRemove = nCount;
    }
    const int nLength = mLength + text.mLength - nRemove;
    const int nOldCapacity = mCapacity;
    mCapacity = nLength + 1;
    char *pBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    strncpy(pBuffer, mBuffer, nPos);
    char *pInsert = &pBuffer[nPos];
    strcpy(pInsert, text.mBuffer);
    strcpy(&pInsert[text.mLength], &mBuffer[nPos + nRemove]);
    mLength = nLength;
    // Yes, the binary frees the old text even when it is the shared empty text.
    MemOrPoolFree(nOldCapacity, mBuffer);
    mBuffer = pBuffer;
    return *this;
}

String &String::Clear() {
    if (mBuffer != g_szStringEmptyBuffer) {
        mLength = 0;
        MemOrPoolFree(mCapacity, mBuffer);
        mBuffer = g_szStringEmptyBuffer;
    }
    return *this;
}

String &String::Truncate(int nLength) {
    if (mBuffer != g_szStringEmptyBuffer) {
        mLength = nLength;
        mBuffer[nLength] = '\0';
    }
    return *this;
}

String &String::Erase(int nPos, int nCount) {
    if (!(static_cast<unsigned int>(nPos + nCount) < static_cast<unsigned int>(mLength))) {
        return Truncate(nPos);
    }
    int i = nPos;
    do {
        mBuffer[i] = mBuffer[i + nCount];
        ++i;
    } while (!(static_cast<unsigned int>(mLength) < static_cast<unsigned int>(i + nCount)));
    mLength -= nCount;
    return *this;
}

String &String::Insert(int nPos, int nCount, char ch) {
    if (mBuffer == g_szStringEmptyBuffer) {
        // Yes, the binary inserts one character into an empty string whatever nCount is.
        mCapacity = 2;
        mBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
        mBuffer[0] = ch;
        mBuffer[1] = '\0';
        mLength = 1;
        return *this;
    }
    const int nLength = mLength + nCount;
    const int nOldCapacity = mCapacity;
    mCapacity = nLength + 1;
    char *pBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    strncpy(pBuffer, mBuffer, nPos);
    unsigned int i = nPos;
    for (; i < static_cast<unsigned int>(nPos + nCount); ++i) {
        pBuffer[i] = ch;
    }
    strcpy(&pBuffer[i], &mBuffer[nPos]);
    mLength = nLength;
    MemOrPoolFree(nOldCapacity, mBuffer);
    mBuffer = pBuffer;
    return *this;
}

String &String::Insert(int nPos, const String &text) {
    if (mBuffer == g_szStringEmptyBuffer) {
        *this = text;
        return *this;
    }
    const int nLength = mLength + text.mLength;
    const int nOldCapacity = mCapacity;
    mCapacity = nLength + 1;
    char *pBuffer = static_cast<char *>(MemOrPoolAlloc(mCapacity, g_pszStringTag));
    strncpy(pBuffer, mBuffer, nPos);
    char *pInsert = &pBuffer[nPos];
    strcpy(pInsert, text.mBuffer);
    strcpy(&pInsert[text.mLength], &mBuffer[nPos]);
    mLength = nLength;
    MemOrPoolFree(nOldCapacity, mBuffer);
    mBuffer = pBuffer;
    return *this;
}

void SetStringReadsText(int bText) {
    g_nStringReadsText = bText;
}

BinStream &operator>>(BinStream &stream, String &text) {
    if (g_nStringReadsText == 0) {
        stream.ReadString(text);
        return stream;
    }
    text.Clear();
    char *pChar = g_szStreamCharScratch;
    while (true) {
        stream.Read(pChar, 1);
        if (*pChar == '\0') {
            text += g_szStreamCharScratch;
            return stream;
        }
        ++pChar;
        if (pChar - g_szStreamCharScratch >= kScratchSize) {
            // Yes, the binary appends the full scratch buffer without a terminator.
            text += g_szStreamCharScratch;
            pChar = g_szStreamCharScratch;
        }
    }
}

#include "utl/Str.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/PoolAlloc.h"

namespace {

// FormatString() formats into a buffer of this size.
const int kFormatStringBufferSize = 1024;

// ReadTextString() collects terminated text in pieces of this size.
const int kTextStringBufferSize = 256;

} // namespace

// 0x1003fda8
static char gEmptyText[] = "";

char *gStringEmptyBuffer = gEmptyText;

// The pool allocator records every buffer under this name.
// 0x100393fc
static const char *gStringBufName = "StringBuf";

// 0x100c13c8
static char gFormatStringBuf[kFormatStringBufferSize];

// Whether ReadTextString() reads terminated text. The control never sets it.
// 0x100c18c8
static bool gReadTextStrings;

// 0x100c17c8
static char gTextStringBuf[kTextStringBufferSize];

char *FormatString(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _vsnprintf(gFormatStringBuf, kFormatStringBufferSize - 1, format, args);
    va_end(args);
    return gFormatStringBuf;
}

BinStream &ReadTextString(BinStream &bs, String &str) {
    if (!gReadTextStrings) {
        return bs >> str;
    }
    str.Clear();
    for (;;) {
        char *c = gTextStringBuf;
        for (;;) {
            bs.Read(c, 1);
            if (*c == '\0') {
                str += gTextStringBuf;
                return bs;
            }
            ++c;
            if (c - gTextStringBuf >= kTextStringBufferSize) {
                break;
            }
        }
        str += gTextStringBuf; // Yes, a full piece has no terminator, and the append reads past it.
    }
}

String::String(const char *str) {
    if (str == NULL) {
        mLen = 0;
        mBuffer = gStringEmptyBuffer;
        return;
    }
    mLen = strlen(str);
    mCapacity = mLen + 1;
    mBuffer = static_cast<char *>(_PoolAlloc(mCapacity, gStringBufName));
    strcpy(mBuffer, str);
}

String::String(const String &other) {
    if (other.mBuffer == gStringEmptyBuffer) {
        mLen = 0;
        mBuffer = gStringEmptyBuffer;
        return;
    }
    mLen = other.mLen;
    mCapacity = mLen + 1;
    mBuffer = static_cast<char *>(_PoolAlloc(mCapacity, gStringBufName));
    strcpy(mBuffer, other.mBuffer);
}

String::~String() {
    if (mBuffer != gStringEmptyBuffer) {
        _PoolFree(mCapacity, mBuffer);
    }
}

void String::Print(const char *str) {
    *this += str;
}

String &String::operator+=(const String &other) {
    if (other.mBuffer == gStringEmptyBuffer) {
        return *this;
    }
    const unsigned int length = mLen + other.mLen;
    const unsigned int oldCapacity = mCapacity;
    mCapacity = length + 1;
    char *buffer = static_cast<char *>(_PoolAlloc(mCapacity, gStringBufName));
    if (mBuffer != gStringEmptyBuffer) {
        strcpy(buffer, mBuffer);
        _PoolFree(oldCapacity, mBuffer);
    }
    strcpy(buffer + mLen, other.mBuffer);
    mLen = length;
    mBuffer = buffer;
    return *this;
}

String &String::operator+=(const char *str) {
    if (str == NULL) {
        return *this;
    }
    const unsigned int length = mLen + strlen(str);
    const unsigned int oldCapacity = mCapacity;
    mCapacity = length + 1;
    char *buffer = static_cast<char *>(_PoolAlloc(mCapacity, gStringBufName));
    if (mBuffer != gStringEmptyBuffer) {
        strcpy(buffer, mBuffer);
        _PoolFree(oldCapacity, mBuffer);
    }
    strcpy(buffer + mLen, str);
    mLen = length;
    mBuffer = buffer;
    return *this;
}

String &String::operator=(const char *str) {
    if (str == mBuffer) {
        return *this;
    }
    if (mBuffer != gStringEmptyBuffer) {
        _PoolFree(mCapacity, mBuffer);
    }
    if (str == NULL) {
        mLen = 0;
        mBuffer = gStringEmptyBuffer;
        return *this;
    }
    mLen = strlen(str);
    mCapacity = mLen + 1;
    mBuffer = static_cast<char *>(_PoolAlloc(mCapacity, gStringBufName));
    strcpy(mBuffer, str);
    return *this;
}

String &String::operator=(const String &other) {
    if (&other == this) {
        return *this;
    }
    if (mBuffer != gStringEmptyBuffer) {
        _PoolFree(mCapacity, mBuffer);
    }
    if (other.mBuffer == gStringEmptyBuffer) {
        mLen = 0;
        mBuffer = gStringEmptyBuffer;
        return *this;
    }
    mLen = strlen(other.mBuffer);
    mCapacity = mLen + 1;
    mBuffer = static_cast<char *>(_PoolAlloc(mCapacity, gStringBufName));
    strcpy(mBuffer, other.mBuffer);
    return *this;
}

bool String::operator!=(const char *str) const {
    if (str == NULL) {
        return true;
    }
    return strcmp(str, mBuffer) != 0;
}

bool String::operator!=(const String &other) const {
    return strcmp(other.mBuffer, mBuffer) != 0;
}

bool String::operator==(const char *str) const {
    if (str == NULL) {
        return false;
    }
    return strcmp(str, mBuffer) == 0;
}

bool String::operator==(const String &other) const {
    return strcmp(other.mBuffer, mBuffer) == 0;
}

bool String::operator<(const String &other) const {
    return strcmp(mBuffer, other.mBuffer) < 0;
}

void String::Resize(unsigned int length) {
    if (mBuffer != gStringEmptyBuffer) {
        _PoolFree(mCapacity, mBuffer);
    }
    mLen = length;
    mCapacity = length + 1;
    mBuffer = static_cast<char *>(_PoolAlloc(mCapacity, gStringBufName));
    memset(mBuffer, 0, mLen + 1);
}

int String::Find(const char *str) const {
    const char *found = strstr(mBuffer, str);
    if (found == NULL) {
        return npos;
    }
    return found - mBuffer;
}

int String::FindLast(char c) const {
    const char *found = strrchr(mBuffer, c);
    if (found == NULL) {
        return npos;
    }
    return found - mBuffer;
}

String String::Substring(unsigned int pos) const {
    ASSERT(pos <= mLen);
    return String(mBuffer + pos);
}

String String::Substring(unsigned int pos, unsigned int count) const {
    ASSERT(pos <= mLen);
    if (pos + count >= mLen) {
        return Substring(pos);
    }
    // The text is built in a local, which the return copies.
    String part;
    part.mLen = count;
    part.mCapacity = count + 1;
    part.mBuffer = static_cast<char *>(_PoolAlloc(part.mCapacity, gStringBufName));
    strncpy(part.mBuffer, mBuffer + pos, count);
    part.mBuffer[count] = '\0';
    return part;
}

String &String::Replace(unsigned int pos, unsigned int count, const String &str) {
    ASSERT(pos <= mLen);
    if (pos + count > mLen) {
        count = mLen - pos;
    }
    const unsigned int oldCapacity = mCapacity;
    const unsigned int length = str.mLen - count + mLen;
    mCapacity = length + 1;
    char *buffer = static_cast<char *>(_PoolAlloc(mCapacity, gStringBufName));
    strncpy(buffer, mBuffer, pos);
    strcpy(buffer + pos, str.mBuffer);
    strcpy(buffer + pos + str.mLen, mBuffer + pos + count);
    mLen = length;
    _PoolFree(oldCapacity, mBuffer); // Yes, even the shared empty buffer is freed.
    mBuffer = buffer;
    return *this;
}

String &String::Clear() {
    if (mBuffer != gStringEmptyBuffer) {
        mLen = 0;
        _PoolFree(mCapacity, mBuffer);
        mBuffer = gStringEmptyBuffer;
    }
    return *this;
}

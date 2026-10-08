#include "utl/BinStream.h"

#include <string.h>

#include "os/Debug.h"
#include "utl/Str.h"

namespace {

// Reverse the bytes of a 2-, 4-, or 8-byte value from one place to another. Both ReadEndian() and
// WriteEndian() expand this, so their failures report the same line.
inline void SwapData(const void *src, void *dst, int bytes) {
    switch (bytes) {
    case 2: {
        const unsigned short s = *static_cast<const unsigned short *>(src);
        *static_cast<unsigned short *>(dst) = static_cast<unsigned short>((s >> 8) | (s << 8));
        break;
    }
    case 4: {
        const unsigned long l = *static_cast<const unsigned long *>(src);
        *static_cast<unsigned long *>(dst) =
            ((l & 0xff0000) >> 8 | l >> 24) | ((l & 0xff00) << 8 | l << 24);
        break;
    }
    case 8: {
        const unsigned __int64 q = *static_cast<const unsigned __int64 *>(src);
        *static_cast<unsigned __int64 *>(dst) = ((q & 0xff) << 56) | ((q & 0xff00) << 40) |
                                                ((q & 0xff0000) << 24) | ((q & 0xff000000) << 8) |
                                                ((q >> 8) & 0xff000000) | ((q >> 24) & 0xff0000) |
                                                ((q >> 40) & 0xff00) | (q >> 56);
        break;
    }
    default:
        ASSERT(0);
        break;
    }
}

} // namespace

BinStream::BinStream(bool littleEndian) : mLittleEndian(littleEndian) {
}

void BinStream::ReadEndian(void *data, int bytes) {
    Read(data, bytes);
    if (!mLittleEndian) {
        SwapData(data, data, bytes);
    }
}

void BinStream::WriteEndian(const void *data, int bytes) {
    if (mLittleEndian) {
        Write(data, bytes);
        return;
    }
    unsigned char swapped[8];
    SwapData(data, swapped, bytes);
    Write(swapped, bytes); // Yes, an unsupported size writes whatever the buffer holds.
}

BinStream &BinStream::operator<<(const char *str) {
    const int length = strlen(str);
    WriteEndian(&length, sizeof(length));
    Write(str, length);
    return *this;
}

BinStream &BinStream::operator>>(String &str) {
    int length;
    ReadEndian(&length, sizeof(length));
    str.Resize(length);
    Read(str.mBuffer, length);
    return *this;
}

void BinStream::ReadString(char *buffer, int bufSize) {
    unsigned int len;
    ReadEndian(&len, sizeof(len));
    ASSERT(len < bufSize);
    Read(buffer, len);
    buffer[len] = '\0';
}

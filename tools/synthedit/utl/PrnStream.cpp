#include "utl/PrnStream.h"

#include <stdarg.h>
#include <stdio.h>

namespace {

// Printf() formats into this buffer, so only one call may run at a time.
const int kPrintfBufferSize = 1024;

} // namespace

// 0x100c3960
static char gPrintfBuffer[kPrintfBufferSize];

void PrnStream::Printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _vsnprintf(gPrintfBuffer, kPrintfBufferSize - 1, format, args);
    va_end(args);
    Print(gPrintfBuffer);
}

PrnStream &PrnStream::operator<<(char c) {
    Printf("%c", c);
    return *this;
}

PrnStream &PrnStream::operator<<(int i) {
    Printf("%d", i);
    return *this;
}

PrnStream &PrnStream::operator<<(float f) {
    Printf("%.2f", f);
    return *this;
}

PrnStream &PrnStream::operator<<(const char *str) {
    Print(str);
    return *this;
}

void PrnStream::Space(int count) {
    while (count != 0) {
        Print(" ");
        --count;
    }
}

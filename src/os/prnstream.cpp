#include "os/prnstream.h"

#include <stdarg.h>
#include <stdio.h>

namespace {

constexpr int kFormatBufferSize = 1024;

} // namespace

// NTSC-U/C: 0x005155f0
static char g_szReportFormatBuffer[kFormatBufferSize];

void PrnStream::Printf(const char *pszFormat, ...) {
    va_list args;
    va_start(args, pszFormat);
    vsprintf(g_szReportFormatBuffer, pszFormat, args);
    va_end(args);
    Print(g_szReportFormatBuffer);
}

PrnStream &PrnStream::operator<<(char ch) {
    Printf("%c", ch);
    return *this;
}

PrnStream &PrnStream::operator<<(short nValue) {
    Printf("%hd", nValue);
    return *this;
}

PrnStream &PrnStream::operator<<(int nValue) {
    Printf("%d", nValue);
    return *this;
}

PrnStream &PrnStream::operator<<(unsigned char nValue) {
    Printf("%u", nValue);
    return *this;
}

PrnStream &PrnStream::operator<<(unsigned short nValue) {
    Printf("%hu", nValue);
    return *this;
}

PrnStream &PrnStream::operator<<(unsigned int nValue) {
    Printf("%u", nValue);
    return *this;
}

PrnStream &PrnStream::operator<<(unsigned long long nValue) {
    // The retail format is %lu, where a long is 64 bits wide.
    Printf("%llu", nValue);
    return *this;
}

PrnStream &PrnStream::operator<<(float fValue) {
    Printf("%.2f", static_cast<double>(fValue));
    return *this;
}

PrnStream &PrnStream::operator<<(const char *pszText) {
    Print(pszText);
    return *this;
}

PrnStream &PrnStream::operator<<(bool bValue) {
    Print(bValue ? "true" : "false");
    return *this;
}

void PrnStream::Space(int nCount) {
    while (nCount != 0) {
        Print(" ");
        --nCount;
    }
}

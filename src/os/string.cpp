#include "os/string.h"

#include <stdarg.h>
#include <stdio.h>

namespace {

constexpr int kFormatBufferSize = 0x400;

// NTSC-U/C: 0x00515a18
char g_szFormatBuffer[kFormatBufferSize] = {};

} // namespace

const char *FormatString(const char *pszFormat, ...) {
    va_list args;
    va_start(args, pszFormat);
    vsprintf(g_szFormatBuffer, pszFormat, args);
    va_end(args);
    return g_szFormatBuffer;
}

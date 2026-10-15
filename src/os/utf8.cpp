#include "os/utf8.h"

#ifdef VIDEO_STANDARD_PAL

#include <cstring>

namespace {

constexpr int kAsciiBytes = 1;
constexpr int kLatinBytes = 2;
constexpr int kTerminatorBytes = 1;

} // namespace

int Utf8Size(const char *pszText) {
    int nSize = 0;
    for (unsigned int i = 0; i < std::strlen(pszText); ++i) {
        nSize += static_cast<signed char>(pszText[i]) < 0 ? kLatinBytes : kAsciiBytes;
    }
    return nSize + kTerminatorBytes;
}

#endif

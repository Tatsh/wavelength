#include "math/hash.h"

namespace {

constexpr int kHashMultiplier = 127;

} // namespace

int HashString(const char *pszText, int nBuckets) {
    int nHash = 0;
    for (const char *p = pszText; *p != 0; ++p) {
        nHash = ((nHash * kHashMultiplier) + static_cast<unsigned char>(*p)) % nBuckets;
    }
    return nHash;
}

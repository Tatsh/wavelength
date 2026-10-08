#include "utl/Hash.h"

namespace {

// Each character scales the hash so far by this factor.
const int kHashMultiplier = 127;

} // namespace

int HashString(const char *str, int tableSize) {
    int hash = 0;
    for (; *str; ++str) {
        hash = ((hash * kHashMultiplier) + static_cast<unsigned char>(*str)) % tableSize;
    }
    return hash;
}

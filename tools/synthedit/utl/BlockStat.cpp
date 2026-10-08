#include "utl/BlockStat.h"

#include <cstring>

bool BlockStat::SizeGreater(const BlockStat &a, const BlockStat &b) {
    return a.mSizeActual >= b.mSizeActual;
}

bool BlockStat::NameLess(const BlockStat &a, const BlockStat &b) {
    return strcmp(a.mName, b.mName) < 0;
}

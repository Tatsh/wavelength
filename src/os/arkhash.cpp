#include "os/arkhash.h"

#include <cstring>

#include "math/hash.h"
#include "os/mem.h"

namespace {

// The slot GetHashValue() reports for an absent name.
constexpr int kNotFound = -1;

} // namespace

ArkHash::ArkHash()
    : mStrings(nullptr), mStringsEnd(nullptr), mStringsLimit(nullptr), mTable(nullptr),
      mTableSize(0) {
}

int ArkHash::GetHashValue(const char *pszName) const {
    int nSlot = HashString(pszName, mTableSize);
    while (mTable[nSlot] != 0) {
        if (strcmp(mStrings + mTable[nSlot], pszName) == 0) {
            return nSlot;
        }
        if (++nSlot == mTableSize) {
            nSlot = 0;
        }
    }
    return kNotFound;
}

void ArkHash::Read(BinStream &stream) {
    PoolMemFree(mStrings);
    PoolMemFree(mTable);
    int nStringsSize;
    stream.ReadEndian(&nStringsSize, sizeof(nStringsSize));
    mStrings = static_cast<char *>(PoolMemAlloc(nStringsSize, "ArkHash", 0));
    mStringsLimit = mStrings + nStringsSize;
    mStringsEnd = mStringsLimit;
    stream.Read(mStrings, nStringsSize);
    stream.ReadEndian(&mTableSize, sizeof(mTableSize));
    mTable = static_cast<int *>(PoolMemAlloc(mTableSize * sizeof(int), "ArkHash", 0));
    // Retail turns each non-zero offset into a pointer here. GetHashValue() adds mStrings instead.
    stream.Read(mTable, mTableSize * sizeof(int));
}

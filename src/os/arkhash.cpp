#include "os/arkhash.h"

#include <cstdint>
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
    while (mTable[nSlot] != nullptr) {
        if (strcmp(mTable[nSlot], pszName) == 0) {
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
    mTable = static_cast<char **>(PoolMemAlloc(mTableSize * sizeof(char *), "ArkHash", 0));
    stream.Read(mTable, mTableSize * sizeof(char *));
    // The stream stores each name as an offset into mStrings, read here into the pointer slot.
    for (char **ppName = mTable; ppName != mTable + mTableSize; ++ppName) {
        if (*ppName != nullptr) {
            *ppName = mStrings + reinterpret_cast<std::uintptr_t>(*ppName);
        }
    }
}

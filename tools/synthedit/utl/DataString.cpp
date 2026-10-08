#include "utl/DataString.h"

#include <cstring>
#include <map>
#include <vector>

#include "os/Debug.h"
#include "utl/Hash.h"

namespace {

// Bytes of interned text.
const int kStringTableBytes = 0x28000;

// Slots of the open-addressing hash table, a prime.
const int kHashTableSize = 0x401b;

} // namespace

// 0x100c3930
static std::vector<const char *> gHashTable;

// 0x100c393c
static char *gStringTableNext;

// 0x100c3940
static std::vector<char> gStringTable;

// 0x100c3950
static std::map<const char *, DataArray *> gDataMacros;

// 0x100c395c
static int gHashEntries;

void DataStringInit() {
    gStringTable.resize(kStringTableBytes);
    gStringTableNext = &gStringTable.front();
    gHashTable.resize(kHashTableSize);
    gHashEntries = 0;
}

void DataStringTerminate() {
    std::map<const char *, DataArray *>::iterator it;
    for (it = gDataMacros.begin(); it != gDataMacros.end(); ++it) {
        it->second->Release();
    }
    gDataMacros.clear();
}

const char *DataFindString(const char *str) {
    if (str >= &gStringTable.front() && str <= &gStringTable.back()) {
        return str;
    }
    const int size = static_cast<int>(gHashTable.size());
    int slot = HashString(str, size);
    const char *entry;
    while ((entry = gHashTable[slot]) != NULL) {
        if (!strcmp(entry, str)) {
            break;
        }
        if (++slot == size) {
            slot = 0;
        }
    }
    return entry;
}

const char *DataAddString(const char *str) {
    ASSERT(gHashEntries < static_cast<int>(gHashTable.size() >> 1));
    const int len = static_cast<int>(strlen(str)) + 1;
    if (gStringTableNext + len >= &gStringTable.front() + gStringTable.size()) {
        TheDebug.Fail("StringTable overrun.");
    }
    strcpy(gStringTableNext, str);
    const int size = static_cast<int>(gHashTable.size());
    int slot = HashString(str, size);
    while (gHashTable[slot]) {
        if (++slot == size) {
            slot = 0;
        }
    }
    gHashTable[slot] = gStringTableNext;
    gStringTableNext += len;
    ++gHashEntries;
    return gHashTable[slot];
}

void DataSetMacro(const char *name, DataArray *macro) {
    const char *symbol = DataFindString(name);
    if (!symbol) {
        symbol = DataAddString(name);
    }
    DataArray *&value = gDataMacros[symbol];
    if (value) {
        value->Release();
    }
    value = macro;
    macro->AddRef();
}

DataArray *DataGetMacro(const char *name) {
    const char *symbol = DataFindString(name);
    if (!symbol) {
        return NULL;
    }
    std::map<const char *, DataArray *>::iterator it = gDataMacros.find(symbol);
    return it == gDataMacros.end() ? NULL : it->second;
}

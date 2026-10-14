#include "script/symbol.h"

#include <map>
#include <string.h>
#include <vector>

#include "math/hash.h"
#include "script/dataarray.h"

namespace {

// The bytes of symbol text the pool takes.
constexpr int kStringPoolSize = 0x28000;

// The slots of the symbol table, a prime.
constexpr int kSymbolTableSize = 16411;

} // namespace

// The globals below are constructed and destroyed by one routine at 0x00297560, which the
// image's static construction and destruction entries at 0x002976b8 and 0x002976d8 call.
// NTSC-U/C: 0x00297560, PAL: 0x002a1170
// NTSC-U/C: 0x002976b8, PAL: 0x002a12c8
// NTSC-U/C: 0x002976d8, PAL: 0x002a12e8

// NTSC-U/C: 0x005152b0
static std::vector<char> g_aStringPool;

// NTSC-U/C: 0x005152c0
static std::vector<const char *> g_aSymbolTable;

// NTSC-U/C: 0x005152d0
static std::map<const char *, DataArray *> g_mapMacros;

// NTSC-U/C: 0x005152e0
static char *g_pszStringPoolWrite;

// NTSC-U/C: 0x005152e4
static int g_nSymbolCount;

void DataReserveSymbols() {
    g_aStringPool.resize(kStringPoolSize, '\0');
    g_pszStringPoolWrite = &g_aStringPool.front();
    g_aSymbolTable.resize(kSymbolTableSize, nullptr);
    g_nSymbolCount = 0;
}

void DataTerminate() {
    for (const auto &macro : g_mapMacros) {
        macro.second->Release();
    }
    g_mapMacros.clear();
}

const char *LookupSymbol(const char *pszText) {
    if (pszText >= &g_aStringPool.front() && pszText <= &g_aStringPool.back()) {
        return pszText;
    }
    int nIndex = HashString(pszText, static_cast<int>(g_aSymbolTable.size()));
    const char *pszEntry = g_aSymbolTable[nIndex];
    while (pszEntry != nullptr && strcmp(pszEntry, pszText) != 0) {
        ++nIndex;
        if (nIndex == static_cast<int>(g_aSymbolTable.size())) {
            nIndex = 0;
        }
        pszEntry = g_aSymbolTable[nIndex];
    }
    return pszEntry;
}

const char *AddSymbol(const char *pszText) {
    const int nLength = static_cast<int>(strlen(pszText)) + 1;
    memcpy(g_pszStringPoolWrite, pszText, nLength);
    int nIndex = HashString(pszText, static_cast<int>(g_aSymbolTable.size()));
    while (g_aSymbolTable[nIndex] != nullptr) {
        ++nIndex;
        if (nIndex == static_cast<int>(g_aSymbolTable.size())) {
            nIndex = 0;
        }
    }
    g_aSymbolTable[nIndex] = g_pszStringPoolWrite;
    g_pszStringPoolWrite = &g_pszStringPoolWrite[nLength];
    ++g_nSymbolCount;
    return g_aSymbolTable[nIndex];
}

void DataArray::DefineMacro(const char *pszName, DataArray *pValue) {
    const char *pszSymbol = LookupSymbol(pszName);
    if (pszSymbol == nullptr) {
        pszSymbol = AddSymbol(pszName);
    }
    DataArray *&pMacro = g_mapMacros[pszSymbol];
    if (pMacro != nullptr) {
        pMacro->Release();
    }
    pMacro = pValue;
    pValue->AddRef();
}

DataArray *DataArray::FindMacro(const char *pszName) {
    const char *pszSymbol = LookupSymbol(pszName);
    if (pszSymbol == nullptr) {
        return nullptr;
    }
    const auto it = g_mapMacros.find(pszSymbol);
    if (it == g_mapMacros.end()) {
        return nullptr;
    }
    return it->second;
}

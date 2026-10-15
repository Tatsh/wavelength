#include "script/datafile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "os/debug.h"
#include "os/filestream.h"
#include "os/fileutil.h"
#include "os/string.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "script/datalex.h"
#include "script/symbol.h"

namespace {

// The nodes an array being parsed grows by.
constexpr int kParseGrowth = 64;

// The delimiters around the file name of an include and of a merge.
constexpr int kIncludeDelimiters = 1;
constexpr int kMergeDelimiters = 2;

// The version byte at the start of a compiled data file.
constexpr char kCompiledVersion = 2;

constexpr int kPathSize = 128;
constexpr int kSymbolSize = 256;

} // namespace

// NTSC-U/C: 0x003b2368, PAL: 0x00431228
static DataArray *g_pDataParseArray = nullptr;

// NTSC-U/C: 0x003b236c, PAL: 0x0043122c
static int g_nDataParseNode = 0;

int g_nDataParseLineNumber = 0;

// NTSC-U/C: 0x003b2374, PAL: 0x00431234
static const char *g_pszDataParseFile = nullptr;

// NTSC-U/C: 0x003b2378, PAL: 0x00431238
static BinStream *g_pDataParseStream = nullptr;

// NTSC-U/C: 0x003b237c, PAL: 0x0043123c
static int g_nDataReadCompiled = 0;

static DataArray *ParseBlock();

DataArray *DataArray::ReadText(const char *pszPath, BinStream *pStream) {
    if (pStream != nullptr) {
        g_pDataParseStream = pStream;
    } else {
        g_pDataParseStream = new FileStream(pszPath, false, true, 0);
        if (g_pDataParseStream->Fail()) {
            DebugPrint("DataReadTxtFile: Can't open %s\n", pszPath);
            delete g_pDataParseStream;
            g_pDataParseStream = nullptr;
            return nullptr;
        }
    }
    g_pszDataParseFile = LookupSymbol(pszPath);
    if (g_pszDataParseFile == nullptr) {
        g_pszDataParseFile = AddSymbol(pszPath);
    }
    g_nDataParseLineNumber = 1;
    DataArray *pArray = ParseBlock();
    if (pStream == nullptr) {
        delete g_pDataParseStream;
        g_pDataParseStream = nullptr;
    }
    return pArray;
}

// NTSC-U/C: 0x002976f8, PAL: 0x002a1308
static BinStream &ReadSymbols(BinStream &stream, std::vector<const char *> &symbols) {
    int nCount;
    stream.ReadEndian(&nCount, sizeof(nCount));
    symbols.resize(nCount, nullptr);
    for (int i = 0; i < nCount; ++i) {
        char szSymbol[kSymbolSize];
        stream.ReadString(szSymbol, kSymbolSize);
        symbols[i] = LookupSymbol(szSymbol);
        if (symbols[i] == nullptr) {
            symbols[i] = AddSymbol(szSymbol);
        }
    }
    return stream;
}

// Read the file a token names between delimiters, relative to the file being parsed, and resume
// parsing that file afterwards.
// NTSC-U/C: 0x002977f8, PAL: 0x002a1408
static DataArray *Include(int nDelimiters) {
    const int nLine = g_nDataParseLineNumber;
    const int nNode = g_nDataParseNode;
    BinStream *pStream = g_pDataParseStream;
    const char *pszFile = g_pszDataParseFile;
    DataArray *pArray = g_pDataParseArray;

    const int nLength = static_cast<int>(strlen(g_pszDataLexText));
    g_pszDataLexText[nLength - nDelimiters] = '\0';
    const char *pszPath = &g_pszDataLexText[nDelimiters];
    if (!FileIsAbsolute(pszPath)) {
        pszPath = FormatString("%s/%s", FileGetPath(g_pszDataParseFile), pszPath);
    }
    DataArray *pResult = DataArray::ReadText(FileNormalizePath(pszPath), nullptr);

    g_pszDataParseFile = pszFile;
    g_nDataParseNode = nNode;
    g_pDataParseArray = pArray;
    g_pDataParseStream = pStream;
    g_nDataParseLineNumber = nLine;
    return pResult;
}

// Turn each \n of quoted text into a newline.
inline void ExpandNewlines(char *pszText) {
    char *p = pszText;
    while (*p != '\0') {
        char *pNext = &p[1];
        if (*p == '\\' && p[1] == 'n') {
            *p = '\n';
            char *q = pNext;
            do {
                *q = q[1];
                ++q;
            } while (*q != '\0');
        }
        p = pNext;
    }
}

// Append the next token to the array being parsed.
// NTSC-U/C: 0x00297918, PAL: 0x002a1528
static bool ParseElement() {
    const int nToken = DataLex();
    if (nToken == kDataTokenEnd || nToken == kDataTokenClose || nToken == kDataTokenDefineEnd) {
        return false;
    }
    if (nToken == kDataTokenMerge) {
        DataArray *pMerge = Include(kMergeDelimiters);
        g_pDataParseArray->Resize(g_nDataParseNode);
        g_pDataParseArray->Merge(pMerge);
        g_nDataParseNode = g_pDataParseArray->Size();
        pMerge->Release();
        return true;
    }
    if (nToken == kDataTokenInclude) {
        DataArray *pInclude = Include(kIncludeDelimiters);
        const int nNeeded = g_nDataParseNode + pInclude->Size();
        if (g_pDataParseArray->Size() < nNeeded) {
            g_pDataParseArray->Resize(nNeeded + kParseGrowth);
        }
        for (int i = 0; i < pInclude->Size(); ++i) {
            const int nNode = g_nDataParseNode++;
            const DataArray::Node value = pInclude->Value(i);
            g_pDataParseArray->SetNode(nNode, value, pInclude->Type(i));
        }
        pInclude->Release();
        return true;
    }
    if (nToken == kDataTokenDefine) {
        DataArray *pValue = ParseBlock();
        --g_nDataParseNode;
        DataArray::DefineMacro(g_pDataParseArray->Sym(g_nDataParseNode), pValue);
        pValue->Release();
        return true;
    }

    if (g_nDataParseNode == g_pDataParseArray->Size()) {
        g_pDataParseArray->Resize(g_nDataParseNode + kParseGrowth);
    }
    DataArray::Node value;
    if (nToken == kDataTokenOpen) {
        value.mArray = ParseBlock();
        const int nNode = g_nDataParseNode++;
        g_pDataParseArray->SetNode(nNode, value, DataArray::kNodeArray);
        value.mArray->Release();
        return true;
    }
    if (nToken == kDataTokenInt) {
        const int nNode = g_nDataParseNode++;
        value.mInt = atoi(g_pszDataLexText);
        g_pDataParseArray->SetNode(nNode, value, DataArray::kNodeInt);
        return true;
    }
    if (nToken == kDataTokenFloat) {
        const int nNode = g_nDataParseNode++;
        value.mFloat = static_cast<float>(atof(g_pszDataLexText));
        g_pDataParseArray->SetNode(nNode, value, DataArray::kNodeFloat);
        return true;
    }
    if (nToken == kDataTokenString || nToken == kDataTokenSymbol) {
        char *pszText;
        if (nToken == kDataTokenSymbol) {
            pszText = g_pszDataLexText;
        } else {
            g_pszDataLexText[g_nDataLexLength - 1] = '\0';
            pszText = &g_pszDataLexText[1];
        }
        ExpandNewlines(pszText);
        value.mSymbol = LookupSymbol(pszText);
        if (value.mSymbol == nullptr) {
            value.mSymbol = AddSymbol(pszText);
            const int nNode = g_nDataParseNode++;
            g_pDataParseArray->SetNode(nNode, value, DataArray::kNodeSymbol);
            return true;
        }
        if (nToken == kDataTokenSymbol) {
            const DataArray *pMacro = DataArray::FindMacro(value.mSymbol);
            if (pMacro != nullptr) {
                for (int i = 0; i < pMacro->Size(); ++i) {
                    const int nNode = g_nDataParseNode++;
                    const DataArray::Node macroValue = pMacro->Value(i);
                    g_pDataParseArray->SetNode(nNode, macroValue, pMacro->Type(i));
                }
                return true;
            }
        }
        const int nNode = g_nDataParseNode++;
        g_pDataParseArray->SetNode(nNode, value, DataArray::kNodeSymbol);
        return true;
    }
    DebugWarn("DataReadFile: Unrecognized token %d (file %s, line %d)",
              nToken,
              g_pszDataParseFile,
              g_nDataParseLineNumber);
    return false;
}

// Parse tokens into a new array until the end of the input or of the enclosing array.
// NTSC-U/C: 0x00297d80, PAL: 0x002a1990
static DataArray *ParseBlock() {
    DataArray *pParent = g_pDataParseArray;
    const int nParentNode = g_nDataParseNode;
    g_pDataParseArray = DataArray::New(kParseGrowth);
    g_pDataParseArray->SetFileLine(g_pszDataParseFile, g_nDataParseLineNumber);
    g_nDataParseNode = 0;
    while (ParseElement()) {
    }
    g_pDataParseArray->Resize(g_nDataParseNode);
    DataArray *pResult = g_pDataParseArray;
    g_pDataParseArray = pParent;
    g_nDataParseNode = nParentNode;
    return pResult;
}

int DataInput(char *pBuffer, int nMaxSize) {
    if (g_pDataParseStream->Fail()) {
        return 0;
    }
    g_pDataParseStream->Read(pBuffer, nMaxSize);
    if (g_pDataParseStream->Fail()) {
        nMaxSize = 0;
    }
    return nMaxSize;
}

void DataSetCompiled() {
    g_nDataReadCompiled = 1;
}

DataArray *DataArray::Read(const char *pszPath, BinStream *pStream) {
    if (!UsingCD() && g_nDataReadCompiled == 0) {
        return ReadText(pszPath, pStream);
    }
    if (pStream != nullptr) {
        g_pDataParseStream = pStream;
    } else {
        char szCompiled[kPathSize];
        MakeCompiledPath(szCompiled, pszPath, true);
        g_pDataParseStream = new FileStream(szCompiled, false, true, 0);
        if (g_pDataParseStream->Fail()) {
            DebugPrint("DataReadFile: Can't open %s\n", szCompiled);
            delete g_pDataParseStream;
            g_pDataParseStream = nullptr;
            return nullptr;
        }
    }
#ifdef VIDEO_STANDARD_PAL
    g_pDataParseStream->EnableReadEncryption();
#endif
    g_pszDataParseFile = LookupSymbol(pszPath);
    if (g_pszDataParseFile == nullptr) {
        g_pszDataParseFile = AddSymbol(pszPath);
    }
    char nVersion;
    g_pDataParseStream->Read(&nVersion, sizeof(nVersion));
    if (nVersion != kCompiledVersion) {
        DebugWarn("DataReadFile. Can't read version %d of file %s\n", nVersion, pszPath);
    }
    std::vector<const char *> files;
    ReadSymbols(*g_pDataParseStream, files);
    DataArray *pArray = new DataArray(*g_pDataParseStream, files);
#ifdef VIDEO_STANDARD_PAL
    g_pDataParseStream->DisableEncryption();
#endif
    if (pStream == nullptr) {
        delete g_pDataParseStream;
        g_pDataParseStream = nullptr;
    }
    return pArray;
}

void DataArray::MakeCompiledPath(char *pszOut, const char *pszPath, bool bCompiled) {
    const bool bUseCompiled = g_nDataReadCompiled != 0 ? true : bCompiled;
    if (UsingCD() || bUseCompiled) {
        const char *pszDirectory = FileGetPath(pszPath);
        const char *pszBase = FileGetBaseName(pszPath);
        sprintf(pszOut, "%s/gen/%s.%s.bin", pszDirectory, pszBase, FileGetExt(pszPath));
    } else {
        strcpy(pszOut, pszPath);
    }
}

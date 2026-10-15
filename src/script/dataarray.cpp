#include "script/dataarray.h"

#include <algorithm>
#include <stdlib.h>
#include <string.h>

#include "script/symbol.h"

namespace {

// Node kinds pack sixteen to a word, two bits each.
constexpr int kTypesPerWordShift = 4;
constexpr int kTypeIndexMask = 15;
constexpr int kTypeBitsShift = 1;
constexpr int kTypeMask = 3;

// The spaces Print() adds for each level of child arrays.
constexpr int kIndentStep = 3;

// FindArray() searches by halves until two candidates remain.
constexpr int kSearchSpan = 2;

// A colour of more than four nodes after the tag has an alpha.
constexpr int kColorAlphaSize = 5;

constexpr int kSymbolBufferSize = 8192;

// The words of node kinds that follow nSize values.
inline int TypeWords(int nSize) {
    return (nSize + kTypeIndexMask) >> kTypesPerWordShift;
}

// The bytes of node storage of nSize nodes.
inline int StorageBytes(int nSize) {
    return (nSize + TypeWords(nSize)) * static_cast<int>(sizeof(DataArray::Node));
}

} // namespace

// NTSC-U/C: 0x003b2364, PAL: 0x00431224
static int g_nDataArrayIndentLevel = 0;

// NTSC-U/C: 0x005132b0
static char g_szDataArrayReadBuf[kSymbolBufferSize];

void DataArray::PrintNode(PrnStream &stream, int nIndex) const {
    switch (Type(nIndex)) {
    case kNodeSymbol:
        stream << '"' << mNodes[nIndex].mSymbol << '"';
        break;
    case kNodeInt:
        stream << mNodes[nIndex].mInt;
        break;
    case kNodeFloat:
        stream << mNodes[nIndex].mFloat;
        break;
    case kNodeArray:
        mNodes[nIndex].mArray->Print(stream);
        break;
    default:
        break;
    }
}

void DataArray::Print(PrnStream &stream) const {
    if (HasArray()) {
        stream << "(\n";
        g_nDataArrayIndentLevel += kIndentStep;
        for (int i = 0; i < mSize; ++i) {
            stream.Space(g_nDataArrayIndentLevel);
            PrintNode(stream, i);
            stream << "\n";
        }
        g_nDataArrayIndentLevel -= kIndentStep;
        stream.Space(g_nDataArrayIndentLevel);
        stream << ")";
    } else {
        stream << "( ";
        for (int i = 0; i < mSize; ++i) {
            PrintNode(stream, i);
            stream << " ";
        }
        stream << ")";
    }
}

bool DataArray::HasArray() const {
    for (int i = 0; i < mSize; ++i) {
        if (Type(i) == kNodeArray) {
            return true;
        }
    }
    return false;
}

DataArray *DataArray::New(int nSize) {
    return new DataArray(nSize);
}

void DataArray::Resize(int nSize) {
    if (nSize == mSize) {
        return;
    }
    for (int i = nSize; i < mSize; ++i) {
        if (Type(i) == kNodeArray) {
            mNodes[i].mArray->Release();
        }
    }
    Node *pOld = mNodes;
    const int nNewTypeWords = TypeWords(nSize);
    mNodes = static_cast<Node *>(MemOrPoolAlloc(StorageBytes(nSize), "Nodes"));
    memset(static_cast<void *>(mNodes), 0, StorageBytes(nSize));
    for (int i = std::min(static_cast<int>(mSize), nSize) - 1; i >= 0; --i) {
        mNodes[i] = pOld[i];
    }
    const int nOldTypeWords = TypeWords(mSize);
    for (int i = std::min(nOldTypeWords, nNewTypeWords) - 1; i >= 0; --i) {
        mNodes[nSize + i] = pOld[mSize + i];
    }
    MemOrPoolFree(StorageBytes(mSize), pOld);
    mSize = static_cast<short>(nSize);
}

void DataArray::AddRef() {
    ++mRefs;
}

void DataArray::Release() {
    if (--mRefs == 0) {
        delete this;
    }
}

DataArray *DataArray::FindArray(int nTag) const {
    if (mSorted != 0) {
        if (mSize == 0) {
            return nullptr;
        }
        int nHigh = mSize - 1;
        int nLow = 0;
        int nLowKey = Array(0)->Value(0).mInt;
        int nHighKey = Array(nHigh)->Value(0).mInt;
        while (nHigh - nLow >= kSearchSpan) {
            const int nMiddle = (nLow + nHigh) >> 1;
            const int nKey = Array(nMiddle)->Value(0).mInt;
            if (nTag < nKey) {
                nHigh = nMiddle;
                nHighKey = nKey;
            } else {
                nLow = nMiddle;
                nLowKey = nKey;
            }
        }
        if (nLowKey == nTag) {
            return Array(nLow);
        }
        if (nHighKey == nTag) {
            return Array(nHigh);
        }
        return nullptr;
    }
    for (int i = 0; i < mSize; ++i) {
        if (Type(i) == kNodeArray && mNodes[i].mArray->Value(0).mInt == nTag) {
            return mNodes[i].mArray;
        }
    }
    return nullptr;
}

DataArray *DataArray::FindArray(const char *pszTag, bool /* bFail */) const {
    // A symbol tag matches as the address of its interned text.
    const char *pszSymbol = LookupSymbol(pszTag);
    if (pszSymbol == nullptr) {
        return nullptr;
    }
    Node tag;
    tag.mSymbol = pszSymbol;
    return FindArray(tag.mInt);
}

bool DataArray::FindSymbol(const char *pszName, const char **ppszValue, bool bFail) const {
    const DataArray *pArray = FindArray(pszName, bFail);
    if (pArray == nullptr) {
        return false;
    }
    *ppszValue = pArray->Sym(1);
    return true;
}

bool DataArray::FindString(const char *pszName, String *pValue, bool bFail) const {
    const DataArray *pArray = FindArray(pszName, bFail);
    if (pArray == nullptr) {
        return false;
    }
    *pValue = pArray->Sym(1);
    return true;
}

bool DataArray::FindInt(const char *pszName, int *pnValue, bool bFail) const {
    const DataArray *pArray = FindArray(pszName, bFail);
    if (pArray == nullptr) {
        return false;
    }
    *pnValue = pArray->Int(1);
    return true;
}

bool DataArray::FindFloat(const char *pszName, float *pfValue, bool bFail) const {
    const DataArray *pArray = FindArray(pszName, bFail);
    if (pArray == nullptr) {
        return false;
    }
    *pfValue = pArray->Float(1);
    return true;
}

bool DataArray::FindBool(const char *pszName, int *pnValue, bool bFail) const {
    const DataArray *pArray = FindArray(pszName, bFail);
    if (pArray == nullptr) {
        return false;
    }
    *pnValue = pArray->Int(1) != 0;
    return true;
}

bool DataArray::FindVector(const char *pszName, Vector2 *pValue, bool bFail) const {
    const DataArray *pArray = FindArray(pszName, bFail);
    if (pArray == nullptr) {
        return false;
    }
    pValue->x = pArray->Float(1);
    pValue->y = pArray->Float(2);
    return true;
}

bool DataArray::FindVector(const char *pszName, Vector3 *pValue, bool bFail) const {
    const DataArray *pArray = FindArray(pszName, bFail);
    if (pArray == nullptr) {
        return false;
    }
    pValue->x = pArray->Float(1);
    pValue->y = pArray->Float(2);
    pValue->z = pArray->Float(3);
    return true;
}

bool DataArray::FindColor(const char *pszName, Color *pValue, bool bFail) const {
    const DataArray *pArray = FindArray(pszName, bFail);
    if (pArray == nullptr) {
        return false;
    }
    pValue->r = pArray->Float(1);
    pValue->g = pArray->Float(2);
    pValue->b = pArray->Float(3);
    if (pArray->mSize >= kColorAlphaSize) {
        pValue->a = pArray->Float(4);
    }
    return true;
}

DataArray::DataArray(int nSize) {
    mFile = nullptr;
    mLine = 0;
    mSorted = 0;
    mRefs = 1;
    mSize = static_cast<short>(nSize);
    mNodes = static_cast<Node *>(MemOrPoolAlloc(StorageBytes(nSize), "Nodes"));
    memset(static_cast<void *>(mNodes), 0, StorageBytes(nSize));
}

DataArray::~DataArray() {
    for (int i = mSize - 1; i >= 0; --i) {
        if (Type(i) == kNodeArray) {
            mNodes[i].mArray->Release();
        }
    }
    MemOrPoolFree(StorageBytes(mSize), mNodes);
}

int DataArray::Type(int nIndex) const {
    const int nWord = mNodes[mSize + (nIndex >> kTypesPerWordShift)].mInt;
    return (nWord >> ((nIndex & kTypeIndexMask) << kTypeBitsShift)) & kTypeMask;
}

const char *DataArray::Sym(int nIndex) const {
    return mNodes[nIndex].mSymbol;
}

DataArray::Node DataArray::Value(int nIndex) const {
    return mNodes[nIndex];
}

int DataArray::Int(int nIndex) const {
    return mNodes[nIndex].mInt;
}

float DataArray::Float(int nIndex) const {
    if (Type(nIndex) != kNodeInt) {
        return mNodes[nIndex].mFloat;
    }
    return static_cast<float>(mNodes[nIndex].mInt);
}

DataArray *DataArray::Array(int nIndex) const {
    return mNodes[nIndex].mArray;
}

void DataArray::SetType(int nIndex, int nType) {
    Node &word = mNodes[mSize + (nIndex >> kTypesPerWordShift)];
    const int nShift = (nIndex & kTypeIndexMask) << kTypeBitsShift;
    word.mInt = (word.mInt & ~(kTypeMask << nShift)) | (nType << nShift);
}

void DataArray::SetNode(int nIndex, Node value, int nType) {
    if (Type(nIndex) == kNodeArray) {
        mNodes[nIndex].mArray->Release();
    }
    if (nType == kNodeSymbol) {
        const char *pszSymbol = LookupSymbol(value.mSymbol);
        value.mSymbol = pszSymbol != nullptr ? pszSymbol : AddSymbol(value.mSymbol);
    }
    SetType(nIndex, nType);
    mNodes[nIndex] = value;
    if (nType == kNodeArray) {
        mNodes[nIndex].mArray->AddRef();
    }
}

DataArray *DataArray::Concat(const std::vector<DataArray *> &arrays, bool bRelease) {
    int nSize = 0;
    for (const DataArray *pArray : arrays) {
        nSize += pArray->mSize;
    }
    DataArray *pResult = New(nSize);
    int nNode = 0;
    for (DataArray *pArray : arrays) {
        for (int i = 0; i < pArray->mSize; ++i) {
            const Node value = pArray->Value(i);
            pResult->SetNode(nNode, value, pArray->Type(i));
            ++nNode;
        }
        if (bRelease) {
            pArray->Release();
        }
    }
    return pResult;
}

void DataArray::Merge(const DataArray *pOther) {
    if (pOther == nullptr) {
        return;
    }
    for (int i = 0; i < pOther->mSize; ++i) {
        if (pOther->Type(i) != kNodeArray) {
            continue;
        }
        const Node child = pOther->Value(i);
        DataArray *pMine = FindArray(child.mArray->Value(0).mInt);
        if (pMine == nullptr) {
            Resize(mSize + 1);
            SetNode(mSize - 1, child, kNodeArray);
        } else {
            pMine->Merge(child.mArray);
        }
    }
}

void DataArray::SetFileLine(const char *pszFile, int nLine) {
    mFile = LookupSymbol(pszFile);
    if (mFile == nullptr) {
        mFile = AddSymbol(pszFile);
    }
    mLine = static_cast<short>(nLine);
}

int DataArray::CompareKeys(const void *pFirst, const void *pSecond) {
    const DataArray *pSecondArray = static_cast<const Node *>(pSecond)->mArray;
    const int nFirst = static_cast<const Node *>(pFirst)->mArray->Value(0).mInt;
    const int nSecond = pSecondArray->Value(0).mInt;
    if (nFirst < nSecond) {
        return -1;
    }
    return nFirst != nSecond;
}

void DataArray::SortByKey() {
    qsort(mNodes, mSize, sizeof(Node), CompareKeys);
    mSorted = 1;
}

DataArray::DataArray(BinStream &stream, const std::vector<const char *> &files) {
    mRefs = 1;
    stream.ReadEndian(&mSize, sizeof(mSize));
    stream.ReadEndian(&mLine, sizeof(mLine));
    stream.ReadEndian(&mSorted, sizeof(mSorted));
    int nFile;
    stream.ReadEndian(&nFile, sizeof(nFile));
    mFile = files[nFile];
    const int nTypeWords = TypeWords(mSize);
    mNodes = static_cast<Node *>(MemOrPoolAlloc(StorageBytes(mSize), "Nodes"));
    memset(static_cast<void *>(mNodes), 0, StorageBytes(mSize));
    stream.Read(&mNodes[mSize], nTypeWords * static_cast<int>(sizeof(Node)));
    for (int i = 0; i < mSize; ++i) {
        const int nType = Type(i);
        // The kind read from the stream is cleared first, so SetNode() releases nothing.
        SetType(i, kNodeInt);
        Node value;
        switch (nType) {
        case kNodeInt:
            stream.ReadEndian(&value.mInt, sizeof(value.mInt));
            SetNode(i, value, kNodeInt);
            break;
        case kNodeSymbol:
            stream.ReadString(g_szDataArrayReadBuf, kSymbolBufferSize);
            value.mSymbol = g_szDataArrayReadBuf;
            SetNode(i, value, kNodeSymbol);
            break;
        case kNodeFloat:
            stream.ReadEndian(&value.mFloat, sizeof(value.mFloat));
            SetNode(i, value, kNodeFloat);
            break;
        case kNodeArray:
            value.mArray = new DataArray(stream, files);
            SetNode(i, value, kNodeArray);
            value.mArray->Release();
            break;
        default:
            break;
        }
    }
}

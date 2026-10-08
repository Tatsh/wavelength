#include "utl/Data.h"

#include <string.h>

#include "os/Debug.h"
#include "utl/DataString.h"
#include "utl/PoolAlloc.h"
#include "utl/Str.h"

namespace {

// Print() indents each level of nested arrays by this many spaces.
const int kIndentStep = 3;

// The types of 16 nodes share one word of the type table.
const int kTypesPerWordShift = 4;
const int kTypeIndexMask = 15;
const int kTypeBits = 2;
const int kTypeMask = 3;

// The longest symbol a compiled script stores, with its terminator.
const int kSymbolBufferSize = 0x2000;

// The words of the nodes and the type table of an array of a size.
inline int NodeWords(int size) {
    return size + ((size + kTypeIndexMask) >> kTypesPerWordShift);
}

// The table of node types has (size + 15) / 16 words.
inline int TypeWordCount(int size) {
    return (size + kTypeIndexMask) >> kTypesPerWordShift;
}

// The pool allocator records node blocks under this name.
const char *const kNodesName = "Nodes";

} // namespace

// The indent of the line Print() is writing.
// 0x100c38e0
static int gDataIndent;

// A symbol the stream constructor is reading.
// 0x100c18e0
static char gDataSymbolBuf[kSymbolBufferSize];

DataArray::DataArray(int size) : mFile(NULL), mLine(0), mSorted(0) {
    mSize = size;
    mRefs = 1;
    const int bytes = NodeWords(size) * sizeof(DataNode);
    mNodes = static_cast<DataNode *>(_PoolAlloc(bytes, kNodesName));
    memset(mNodes, 0, bytes);
}

DataArray::DataArray(BinStream &bs, std::vector<const char *> &filenames) {
    mRefs = 1;
    bs.ReadEndian(&mSize, sizeof(mSize));
    bs.ReadEndian(&mLine, sizeof(mLine));
    bs.ReadEndian(&mSorted, sizeof(mSorted));
    unsigned int fileIdx;
    bs.ReadEndian(&fileIdx, sizeof(fileIdx));
    ASSERT(fileIdx < filenames.size());
    mFile = filenames[fileIdx];
    const int typeWords = TypeWordCount(mSize);
    const int bytes = (mSize + typeWords) * sizeof(DataNode);
    mNodes = static_cast<DataNode *>(_PoolAlloc(bytes, kNodesName));
    memset(mNodes, 0, bytes);
    bs.Read(TypeWords(), typeWords * sizeof(int)); // The type table is not byte-swapped.
    for (int i = 0; i < mSize; ++i) {
        const int type = Type(i);
        // The node is still empty; SetNode() must not release what the type claims it holds.
        SetType(i, kDataInt);
        DataNode value;
        switch (type) {
        case kDataInt:
            bs.ReadEndian(&value.i, sizeof(value.i));
            SetNode(i, value, kDataInt);
            break;
        case kDataSymbol:
            bs.ReadString(gDataSymbolBuf, kSymbolBufferSize);
            value.sym = gDataSymbolBuf;
            SetNode(i, value, kDataSymbol);
            break;
        case kDataFloat:
            bs.ReadEndian(&value.f, sizeof(value.f));
            SetNode(i, value, kDataFloat);
            break;
        case kDataArray:
            value.array = new DataArray(bs, filenames);
            SetNode(i, value, kDataArray);
            value.array->Release();
            break;
        }
    }
}

DataArray::~DataArray() {
    for (int i = mSize - 1; i >= 0; --i) {
        if (Type(i) == kDataArray) {
            mNodes[i].array->Release();
        }
    }
    _PoolFree(NodeWords(mSize) * sizeof(DataNode), mNodes);
}

void *DataArray::operator new(size_t size) {
    return PoolAlloc(sizeof(DataArray), size, "DataArray", 0);
}

void DataArray::operator delete(void *mem) {
    PoolFree(sizeof(DataArray), mem);
}

void DataArray::PrintNode(PrnStream &s, int i) {
    switch (Type(i)) {
    case kDataInt:
        s << mNodes[i].i;
        break;
    case kDataSymbol:
        s << '"' << mNodes[i].sym << '"';
        break;
    case kDataFloat:
        s << mNodes[i].f;
        break;
    case kDataArray:
        mNodes[i].array->Print(s);
        break;
    }
}

void DataArray::Print(PrnStream &s) {
    int i;
    if (HasArrays()) {
        s << "(\n";
        gDataIndent += kIndentStep;
        for (i = 0; i < mSize; ++i) {
            s.Space(gDataIndent);
            PrintNode(s, i);
            s << "\n";
        }
        gDataIndent -= kIndentStep;
        s.Space(gDataIndent);
        s << ")";
        return;
    }
    s << "( ";
    for (i = 0; i < mSize; ++i) {
        PrintNode(s, i);
        s << " ";
    }
    s << ")";
}

bool DataArray::HasArrays() {
    for (int i = 0; i < mSize; ++i) {
        if (Type(i) == kDataArray) {
            return true;
        }
    }
    return false;
}

DataArray *NewDataArray(int size) {
    return new DataArray(size);
}

void DataArray::Resize(int size) {
    if (size == mSize) {
        return;
    }
    int i;
    for (i = size; i < mSize; ++i) {
        if (Type(i) == kDataArray) {
            mNodes[i].array->Release();
        }
    }
    DataNode *oldNodes = mNodes;
    const int newTypeWords = TypeWordCount(size);
    const int bytes = (size + newTypeWords) * sizeof(DataNode);
    mNodes = static_cast<DataNode *>(_PoolAlloc(bytes, kNodesName));
    memset(mNodes, 0, bytes);
    const int keptNodes = size < mSize ? size : mSize;
    for (i = keptNodes - 1; i >= 0; --i) {
        mNodes[i] = oldNodes[i];
    }
    const int oldTypeWords = TypeWordCount(mSize);
    const int keptTypeWords = newTypeWords < oldTypeWords ? newTypeWords : oldTypeWords;
    for (i = keptTypeWords - 1; i >= 0; --i) {
        mNodes[size + i] = oldNodes[mSize + i];
    }
    _PoolFree(NodeWords(mSize) * sizeof(DataNode), oldNodes);
    mSize = size;
}

void DataArray::AddRef() {
    ++mRefs;
}

void DataArray::Release() {
    if (--mRefs == 0) {
        delete this;
    }
}

DataArray *DataArray::FindArray(int tag) {
    int i;
    if (mSorted == 0) {
        for (i = 0; i < mSize; ++i) {
            if (Type(i) == kDataArray && mNodes[i].array->Node(0).i == tag) {
                return mNodes[i].array;
            }
        }
        return NULL;
    }
    if (mSize == 0) {
        return NULL;
    }
    int low = 0;
    int high = mSize - 1;
    int lowTag = Array(low)->Node(0).i;
    int highTag = Array(high)->Node(0).i;
    while (high - low > 1) {
        const int middle = (high + low) >> 1;
        const int middleTag = Array(middle)->Node(0).i;
        if (tag < middleTag) {
            high = middle;
            highTag = middleTag;
        } else {
            low = middle;
            lowTag = middleTag;
        }
    }
    if (lowTag == tag) {
        return Array(low);
    }
    if (highTag == tag) {
        return Array(high);
    }
    return NULL;
}

DataArray *DataArray::FindArray(const char *tag, bool fail) {
    ASSERT(tag);
    const char *sym = DataFindString(tag);
    DataArray *found = NULL;
    if (sym != NULL) {
        // A symbol is compared by its address in the string table.
        found = FindArray(reinterpret_cast<int>(sym));
    }
    if (found == NULL && fail) {
        TheDebug.Fail("Couldn't find '%s' in array (file %s, line %d)", tag, mFile, mLine);
    }
    return found;
}

bool DataArray::FindSymbol(const char *tag, const char **value, bool fail) {
    DataArray *found = FindArray(tag, fail);
    if (found == NULL) {
        return false;
    }
    *value = found->Sym(1);
    return true;
}

bool DataArray::FindInt(const char *tag, int *value, bool fail) {
    DataArray *found = FindArray(tag, fail);
    if (found == NULL) {
        return false;
    }
    *value = found->Int(1);
    return true;
}

bool DataArray::FindFloat(const char *tag, float *value, bool fail) {
    DataArray *found = FindArray(tag, fail);
    if (found == NULL) {
        return false;
    }
    *value = found->Float(1);
    return true;
}

bool DataArray::FindBool(const char *tag, bool *value, bool fail) {
    DataArray *found = FindArray(tag, fail);
    if (found == NULL) {
        return false;
    }
    *value = found->Int(1) != 0;
    return true;
}

int DataArray::Type(int i) {
    ASSERT(i < mSize);
    const int word = TypeWords()[i >> kTypesPerWordShift];
    return (word >> ((i & kTypeIndexMask) * kTypeBits)) & kTypeMask;
}

const char *DataArray::Sym(int i) {
    if (Type(i) != kDataSymbol) {
        String text;
        PrintNode(text, i);
        TheDebug.Fail(
            "Data %s is not String (file %s, line %d, node %d)", text.c_str(), mFile, mLine, i);
    }
    return mNodes[i].sym;
}

DataArray::DataNode DataArray::Node(int i) {
    ASSERT(i < mSize);
    return mNodes[i];
}

int DataArray::Int(int i) {
    if (Type(i) != kDataInt) {
        String text;
        PrintNode(text, i);
        TheDebug.Fail(
            "Data %s is not Integer (file %s, line %d, node %d)", text.c_str(), mFile, mLine, i);
    }
    return mNodes[i].i;
}

float DataArray::Float(int i) {
    if (Type(i) == kDataInt) {
        return static_cast<float>(mNodes[i].i);
    }
    if (Type(i) != kDataFloat) {
        String text;
        PrintNode(text, i);
        TheDebug.Fail(
            "Data %s is not Real (file %s, line %d, node %d)", text.c_str(), mFile, mLine, i);
    }
    return mNodes[i].f;
}

DataArray *DataArray::Array(int i) {
    if (Type(i) != kDataArray) {
        String text;
        PrintNode(text, i);
        TheDebug.Fail(
            "Data %s is not Array (file %s, line %d, node %d)", text.c_str(), mFile, mLine, i);
    }
    return mNodes[i].array;
}

void DataArray::SetType(int i, int type) {
    int &word = TypeWords()[i >> kTypesPerWordShift];
    const int shift = (i & kTypeIndexMask) * kTypeBits;
    word = (word & ~(kTypeMask << shift)) | (type << shift);
}

void DataArray::SetNode(int i, DataNode value, int type) {
    ASSERT(i < mSize);
    if (Type(i) == kDataArray) {
        mNodes[i].array->Release();
    }
    if (type == kDataSymbol) {
        value.sym = DataInternString(value.sym);
    }
    SetType(i, type);
    mNodes[i] = value;
    if (type == kDataArray) {
        mNodes[i].array->AddRef();
    }
}

void DataMergeTags(DataArray *dest, DataArray *src) {
    ASSERT(dest);
    if (src == NULL) {
        return;
    }
    for (int i = 0; i < src->Size(); ++i) {
        if (src->Type(i) != DataArray::kDataArray) {
            continue;
        }
        DataArray::DataNode node = src->Node(i);
        DataArray *found = dest->FindArray(node.array->Node(0).i);
        if (found == NULL) {
            dest->Resize(dest->Size() + 1);
            dest->SetNode(dest->Size() - 1, node, DataArray::kDataArray);
        } else {
            DataMergeTags(found, node.array);
        }
    }
}

void DataArray::SetFileLine(const char *file, int line) {
    mFile = DataInternString(file);
    mLine = line;
}

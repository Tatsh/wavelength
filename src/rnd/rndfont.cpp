#include "rnd/rndfont.h"

#include "os/debug.h"
#include "rnd/rndbitmap.h"
#include "rnd/rndmanager.h"
#include "rnd/rndtex.h"

namespace {

// The first version with a material and a grid, with float grid sizes, and with characters.
constexpr int kRevGrid = 1;
constexpr int kRevFloatGrid = 2;
constexpr int kRevChars = 2;
constexpr int kRevNoLegacyFields = 3;

// The characters a font of an early version has.
const char *const kDefaultChars =
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    "abcdefghijklmnopqrstuvwxyz{|}~";

// The access CellInfo() requests of the texture's bitmap.
constexpr int kLockBitmapFlags = 3;

// The share of a cell an empty character spans.
constexpr int kEmptyCellDivisor = 4;

// A legacy per-character material record that Load() reads and discards. The RTTI names it
// MatChar.
struct MatChar {
    RndMat *mMat;
    int mParams[2];
};

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.mLength == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

// NTSC-U/C: 0x00226278, PAL: 0x0022f030
BinStream &operator>>(BinStream &stream, MatChar &matChar) {
    matChar.mMat = FindByName<RndMat>(stream);
    stream.ReadEndian(&matChar.mParams[0], sizeof(matChar.mParams[0]));
    stream.ReadEndian(&matChar.mParams[1], sizeof(matChar.mParams[1]));
    return stream;
}

// NTSC-U/C: 0x00382110, PAL: 0x003f0828
void SkipMatChars(BinStream &stream) {
    std::map<char, MatChar> matChars;
    int nCount;
    stream.ReadEndian(&nCount, sizeof(nCount));
    for (int i = 0; i < nCount; ++i) {
        char ch;
        stream.Read(&ch, sizeof(ch));
        stream >> matChars[ch];
    }
}

bool HasOpaquePixel(const RndBitmap &bitmap, int nX, int nYStart, int nYEnd) {
    for (int y = nYStart; y < nYEnd; ++y) {
        unsigned char nRed;
        unsigned char nGreen;
        unsigned char nBlue;
        unsigned char nAlpha;
        bitmap.PixelColor(nX, y, nRed, nGreen, nBlue, nAlpha);
        if (nAlpha != 0) {
            return true;
        }
    }
    return false;
}

} // namespace

const char *RndFont::sClassName = "Font";
int RndFont::sRev = 3;

RndFont::RndFont(const char *pszName) : RndObject(pszName) {
    mRows = 1.0f;
    mSize = 0.0f;
    mMat = nullptr;
    mCols = 1.0f;
    mSpace = 0.0f;
}

RndFont::~RndFont() {
    ReleaseRefs();
}

void RndFont::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndFont]\n";
    stream << "mat:" << static_cast<const RndObject *>(mMat) << " rows:" << mRows
           << " cols:" << mCols << "\n";
    stream << "size:" << mSize << " space:" << mSpace << "\n";
    stream << "chars:";
    stream.Print(mChars.c_str());
    stream << "\n";
}

void RndFont::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    stream.WriteString(mMat != nullptr ? mMat->mName.c_str() : "");
    stream.WriteEndian(&mRows, sizeof(mRows));
    stream.WriteEndian(&mCols, sizeof(mCols));
    stream.WriteEndian(&mSize, sizeof(mSize));
    stream.WriteEndian(&mSpace, sizeof(mSpace));
    stream.WriteString(mChars.c_str());
}

void RndFont::Replace(RndObject *pFrom, RndObject *pTo) {
    if (mMat != pFrom) {
        return;
    }
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
    if (mMat != nullptr) {
        mMat = pTo != nullptr ? dynamic_cast<RndMat *>(pTo) : nullptr;
    }
    if (mMat != nullptr) {
        mMat->AddRef(this);
    }
}

void RndFont::Copy(const RndObject *pSource, [[maybe_unused]] int nFlags) {
    const RndFont *pFont = pSource != nullptr ? dynamic_cast<const RndFont *>(pSource) : nullptr;
    ReleaseRefs();
    mMat = pFont->mMat;
    mRows = pFont->mRows;
    mCols = pFont->mCols;
    mSize = pFont->mSize;
    mSpace = pFont->mSpace;
    mChars = pFont->mChars;
    BuildCharMap();
}

void RndFont::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new Font");
        return;
    }
    ReleaseRefs();
    if (nRev < kRevNoLegacyFields) {
        String unusedName;
        int nUnused;
        stream.ReadEndian(&nUnused, sizeof(nUnused));
        stream.ReadEndian(&nUnused, sizeof(nUnused));
        stream.ReadEndian(&nUnused, sizeof(nUnused));
        unsigned char nUnusedFlag;
        stream.Read(&nUnusedFlag, sizeof(nUnusedFlag));
        stream.ReadEndian(&nUnused, sizeof(nUnused));
        stream >> unusedName;
    }
    if (nRev < kRevGrid) {
        SkipMatChars(stream);
    } else {
        mMat = FindByName<RndMat>(stream);
        if (nRev < kRevFloatGrid) {
            int nRows;
            stream.ReadEndian(&nRows, sizeof(nRows));
            int nCols;
            stream.ReadEndian(&nCols, sizeof(nCols));
            mRows = static_cast<float>(nRows);
            mCols = static_cast<float>(nCols);
        } else {
            stream.ReadEndian(&mRows, sizeof(mRows));
            stream.ReadEndian(&mCols, sizeof(mCols));
        }
        stream.ReadEndian(&mSize, sizeof(mSize));
        stream.ReadEndian(&mSpace, sizeof(mSpace));
    }
    if (nRev >= kRevChars) {
        stream >> mChars;
    } else {
        mChars = kDefaultChars;
    }
    BuildCharMap();
}

void RndFont::Set(
    RndMat *pMat, const char *pszChars, float fRows, float fCols, float fSize, float fSpace) {
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
    mMat = pMat; // Yes, the binary takes the reference in BuildCharMap().
    if (fRows < 1.0f) {
        fRows = 1.0f;
    }
    if (fCols < 1.0f) {
        fCols = 1.0f;
    }
    mRows = fRows;
    mCols = fCols;
    mSize = fSize;
    mSpace = fSpace;
    mChars = pszChars;
    BuildCharMap();
}

void RndFont::BuildCharMap() {
    if (mMat != nullptr) {
        mMat->AddRef(this);
    }
    int nRow = 0;
    int nCol = 0;
    for (const char *pChar = mChars.c_str(); pChar != mChars.c_str() + mChars.mLength; ++pChar) {
        if (nRow >= static_cast<int>(mRows)) {
            DebugPrint("%s: too many characters\n", mName.c_str());
            return;
        }
        mCharMap[*pChar] = CellInfo(nRow, nCol);
        ++nCol;
        if (nCol >= static_cast<int>(mCols)) {
            nCol = 0;
            ++nRow;
        }
    }
}

RndFont::CharInfo RndFont::CellInfo(int nRow, int nCol) const {
    CharInfo info;
    RndTex *pTex = nullptr;
    if (mMat != nullptr && !mMat->mStages.empty()) {
        pTex = mMat->mStages[0].mTex;
    }
    if (pTex == nullptr) {
        info.mWidth = 0.0f;
        info.mUvStart.y = 0.0f;
        info.mUvStart.x = 0.0f;
        info.mUvEnd.y = 0.0f;
        info.mUvEnd.x = 0.0f;
        return info;
    }
    const float fRow = static_cast<float>(nRow);
    const float fCol = static_cast<float>(nCol);
    const RndBitmap *pBitmap = pTex->LockBitmap(kLockBitmapFlags);
    const int nWidth = pBitmap->mWidth;
    const int nHeight = pBitmap->mHeight;
    const auto nXStart = static_cast<int>(static_cast<float>(nCol * nWidth) / mCols);
    const auto nXEnd = static_cast<int>(static_cast<float>(nCol * nWidth + nWidth) / mCols);
    const auto nYStart = static_cast<int>(static_cast<float>(nRow * nHeight) / mRows);
    const auto nYEnd = static_cast<int>(static_cast<float>(nRow * nHeight + nHeight) / mRows);
    const int nCellWidth = nXEnd - nXStart;

    int nFirst = nXStart;
    const int nStep = nXStart < nXEnd ? 1 : -1;
    while (nFirst != nXEnd && !HasOpaquePixel(*pBitmap, nFirst, nYStart, nYEnd)) {
        nFirst += nStep;
    }
    int nLast = nXEnd - 1;
    const int nLastStep = nLast < nXStart - 1 ? 1 : -1;
    while (nLast != nXStart - 1 && !HasOpaquePixel(*pBitmap, nLast, nYStart, nYEnd)) {
        nLast += nLastStep;
    }
    pTex->UnlockBitmap();

    int nOpaqueStart = nFirst;
    int nOpaqueEnd = nLast + 1;
    if (nFirst >= nOpaqueEnd) {
        nOpaqueStart = nXStart;
        nOpaqueEnd = nXStart + nCellWidth / kEmptyCellDivisor;
    }
    const auto fOpaqueWidth = static_cast<float>(nOpaqueEnd - nOpaqueStart);
    const auto fCellWidth = static_cast<float>(nCellWidth);
    const auto fOffset = static_cast<float>(nOpaqueStart - nXStart);
    info.mWidth = mSize * fOpaqueWidth / fCellWidth;
    info.mUvStart.x = (fCol + fOffset / fCellWidth) / mCols;
    info.mUvStart.y = fRow / mRows;
    const float fUvWidth = fOpaqueWidth / fCellWidth / mCols;
    const float fUvHeight = 1.0f / mRows;
    info.mUvEnd.x = info.mUvStart.x + fUvWidth;
    info.mUvEnd.y = info.mUvStart.y + fUvHeight;
    return info;
}

void RndFont::CharUv(char ch, Vector2 &uvStart, Vector2 &uvEnd) const {
    const auto it = mCharMap.find(ch);
    if (it == mCharMap.end()) {
        uvStart.x = 0.0f;
        uvStart.y = 0.0f;
        uvEnd.x = 0.0f;
        uvEnd.y = 0.0f;
        return;
    }
    uvStart = it->second.mUvStart;
    uvEnd = it->second.mUvEnd;
}

float RndFont::CharWidth(char ch) const {
    const auto it = mCharMap.find(ch);
    return it == mCharMap.end() ? 0.0f : it->second.mWidth;
}

void RndFont::ReleaseRefs() {
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
}

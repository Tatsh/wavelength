#include "rnd/rndbitmap.h"

#include <cstdint>
#include <cstring>

#include "os/debug.h"
#include "os/file.h"
#include "os/mem.h"

namespace {

// The GS block order of the bytes of an 8-bit and a 4-bit block, for even and odd block rows. A
// 4-bit entry is the byte offset doubled, plus 1 for the high half of the byte.
const unsigned char g_abGsSwizzle8Even[] = {
    0,  4,  8,  12, 16, 20, 24, 28, 2,  6,  10, 14, 18, 22, 26, 30, 32, 36, 40, 44, 48, 52,
    56, 60, 34, 38, 42, 46, 50, 54, 58, 62, 17, 21, 25, 29, 1,  5,  9,  13, 19, 23, 27, 31,
    3,  7,  11, 15, 49, 53, 57, 61, 33, 37, 41, 45, 51, 55, 59, 63, 35, 39, 43, 47,
};
const unsigned char g_abGsSwizzle8Odd[] = {
    16, 20, 24, 28, 0,  4,  8,  12, 18, 22, 26, 30, 2,  6,  10, 14, 48, 52, 56, 60, 32, 36,
    40, 44, 50, 54, 58, 62, 34, 38, 42, 46, 1,  5,  9,  13, 17, 21, 25, 29, 3,  7,  11, 15,
    19, 23, 27, 31, 33, 37, 41, 45, 49, 53, 57, 61, 35, 39, 43, 47, 51, 55, 59, 63,
};
const unsigned char g_abGsSwizzle4Even[] = {
    0,   8,   16,  24,  32,  40,  48,  56,  2,   10,  18,  26,  34,  42, 50,  58,  4,   12,  20,
    28,  36,  44,  52,  60,  6,   14,  22,  30,  38,  46,  54,  62,  64, 72,  80,  88,  96,  104,
    112, 120, 66,  74,  82,  90,  98,  106, 114, 122, 68,  76,  84,  92, 100, 108, 116, 124, 70,
    78,  86,  94,  102, 110, 118, 126, 33,  41,  49,  57,  1,   9,   17, 25,  35,  43,  51,  59,
    3,   11,  19,  27,  37,  45,  53,  61,  5,   13,  21,  29,  39,  47, 55,  63,  7,   15,  23,
    31,  97,  105, 113, 121, 65,  73,  81,  89,  99,  107, 115, 123, 67, 75,  83,  91,  101, 109,
    117, 125, 69,  77,  85,  93,  103, 111, 119, 127, 71,  79,  87,  95,
};
const unsigned char g_abGsSwizzle4Odd[] = {
    32,  40,  48,  56,  0,   8,   16,  24,  34,  42, 50,  58,  2,   10,  18,  26,  36,  44, 52,
    60,  4,   12,  20,  28,  38,  46,  54,  62,  6,  14,  22,  30,  96,  104, 112, 120, 64, 72,
    80,  88,  98,  106, 114, 122, 66,  74,  82,  90, 100, 108, 116, 124, 68,  76,  84,  92, 102,
    110, 118, 126, 70,  78,  86,  94,  1,   9,   17, 25,  33,  41,  49,  57,  3,   11,  19, 27,
    35,  43,  51,  59,  5,   13,  21,  29,  37,  45, 53,  61,  7,   15,  23,  31,  39,  47, 55,
    63,  65,  73,  81,  89,  97,  105, 113, 121, 67, 75,  83,  91,  99,  107, 115, 123, 69, 77,
    85,  93,  101, 109, 117, 125, 71,  79,  87,  95, 103, 111, 119, 127,
};

// The bits per pixel at and below which a bitmap has a palette, and the size of an entry.
constexpr int kMaxPaletteBpp = 8;
constexpr int kPaletteEntryBytes = 4;

constexpr int kBpp4 = 4;
constexpr int kBpp8 = 8;
constexpr int kBpp16 = 16;
constexpr int kBpp24 = 24;
constexpr int kBpp32 = 32;

// The smallest sizes the GS block order applies to.
constexpr int kMinSwizzle8Width = 16;
constexpr int kMinSwizzle8Height = 16;
constexpr int kMinSwizzle4Width = 32;
constexpr int kMinSwizzle4Height = 16;

// The size a 4-bit bitmap is laid out in pages beyond.
constexpr int kSwizzle4PageSize = 128;

// The largest sum of component differences, which every palette entry beats.
constexpr int kMaxColorDistance = 1024;

// The bits of a 16-bit pixel.
constexpr unsigned int kAlpha16Bit = 0x8000;
constexpr unsigned int kRed16Mask = 0x7c00;
constexpr unsigned int kGreen16Mask = 0x3e0;
constexpr unsigned int kComponent16Mask = 0xf8;
constexpr unsigned int kAlpha8Bit = 0x80;

constexpr unsigned char kOpaque = 255;

// The palette index bits the GS order of an 8-bit palette exchanges.
constexpr int kGsPaletteSwapMask = 0x18;
constexpr int kGsPaletteSwapLow = 8;
constexpr int kGsPaletteSwapHigh = 0x10;

struct Header {
    unsigned char mRev;
    unsigned char mBpp;
    unsigned char mOrder;
    unsigned char mNumMips;
    unsigned short mWidth;
    unsigned short mHeight;
    unsigned short mRowBytes;
    unsigned char mUnused[6];
};

constexpr std::uintptr_t kLoadAlignment = 16;

// The BMP file headers after the two-byte signature.
struct BmpFileHeader {
    unsigned int mSize;
    unsigned short mReserved1;
    unsigned short mReserved2;
    unsigned int mOffBits;
};

struct BmpInfoHeader {
    unsigned int mSize;
    int mWidth;
    int mHeight;
    unsigned short mPlanes;
    unsigned short mBitCount;
    unsigned int mCompression;
    unsigned int mSizeImage;
    int mXPelsPerMeter;
    int mYPelsPerMeter;
    unsigned int mClrUsed;
    unsigned int mClrImportant;
};

constexpr unsigned short kBmpSignature = 0x4d42;
constexpr int kBmpMinBitCount = 4;
constexpr int kBmpPelsPerMeter = 2834;
constexpr int kBmpRowAlignment = 4;
constexpr int kFileModeRead = 1;
constexpr int kFileModeWrite = 0x602;

// The treatments a BMP file name requests.
enum BmpAlphaMode {
    kBmpAlphaNone = 0,
    kBmpAlphaTransparentBlack = 1,
    kBmpAlphaGrayWhite = 2,
    kBmpAlphaGrayAlpha = 3,
};

void ApplyBmpAlphaMode(int nMode,
                       unsigned char &nRed,
                       unsigned char &nGreen,
                       unsigned char &nBlue,
                       unsigned char &nAlpha) {
    switch (nMode) {
    case kBmpAlphaGrayWhite:
        nBlue = kOpaque;
        nAlpha = nRed;
        nGreen = kOpaque;
        nRed = kOpaque;
        break;
    case kBmpAlphaTransparentBlack:
        if ((nBlue & nRed & nGreen) == 0) { // Yes, the binary tests the bitwise and of the three.
            nAlpha = 0;
        }
        break;
    case kBmpAlphaGrayAlpha:
        nAlpha = nRed;
        break;
    default:
        break;
    }
}

unsigned int ReadWord(const unsigned char *pData) {
    unsigned int nWord;
    std::memcpy(&nWord, pData, sizeof(nWord));
    return nWord;
}

unsigned short ReadHalf(const unsigned char *pData) {
    unsigned short nHalf;
    std::memcpy(&nHalf, pData, sizeof(nHalf));
    return nHalf;
}

} // namespace

unsigned char RndBitmap::sRev = 0;

int RndBitmap::NumMips() const {
    int nCount = 0;
    for (const RndBitmap *pMip = mMip; pMip != nullptr; pMip = pMip->mMip) {
        ++nCount;
    }
    return nCount;
}

int RndBitmap::PixelBytes() const {
    return mRowBytes * mHeight;
}

int RndBitmap::PaletteBytes() const {
    return mBpp <= kMaxPaletteBpp ? kPaletteEntryBytes << mBpp : 0;
}

void RndBitmap::NormalizeLayout() {
    if (mRowBytes == 0) {
        int nWidth = mWidth;
        if (mBpp == kBpp4 && (mWidth & 1) != 0) {
            nWidth = mWidth + 1;
        }
        mRowBytes = (nWidth * mBpp) >> 3;
    }
    if ((mOrder & kOrderSwizzled) == 0) {
        return;
    }
    bool bClear;
    if (mBpp == kBpp8) {
        bClear = mWidth < kMinSwizzle8Width || mHeight < kMinSwizzle8Height;
    } else if (mBpp == kBpp4) {
        bClear = mWidth < kMinSwizzle4Width || mHeight < kMinSwizzle4Height;
    } else {
        bClear = mBpp > kMaxPaletteBpp;
    }
    if (bClear) {
        mOrder &= ~kOrderSwizzled;
    }
}

unsigned char RndBitmap::NearestColor(unsigned char nRed,
                                      unsigned char nGreen,
                                      unsigned char nBlue,
                                      unsigned char nAlpha) const {
    int nBest = kMaxColorDistance;
    int nBestIndex = -1;
    for (int i = (1 << mBpp) - 1; i >= 0; --i) {
        unsigned char nEntryRed;
        unsigned char nEntryGreen;
        unsigned char nEntryBlue;
        unsigned char nEntryAlpha;
        PaletteColor(i, nEntryRed, nEntryGreen, nEntryBlue, nEntryAlpha);
        int nRedDiff = nEntryRed - nRed;
        int nGreenDiff = nEntryGreen - nGreen;
        int nBlueDiff = nEntryBlue - nBlue;
        int nAlphaDiff = nEntryAlpha - nAlpha;
        nRedDiff = nRedDiff < 0 ? -nRedDiff : nRedDiff;
        nGreenDiff = nGreenDiff < 0 ? -nGreenDiff : nGreenDiff;
        nBlueDiff = nBlueDiff < 0 ? -nBlueDiff : nBlueDiff;
        nAlphaDiff = nAlphaDiff < 0 ? -nAlphaDiff : nAlphaDiff;
        const int nDistance = nRedDiff + nGreenDiff + nBlueDiff + nAlphaDiff;
        if (nDistance < nBest) {
            nBest = nDistance;
            nBestIndex = i;
        }
    }
    return static_cast<unsigned char>(nBestIndex);
}

void RndBitmap::ReadColor(const unsigned char *pData,
                          unsigned char &nRed,
                          unsigned char &nGreen,
                          unsigned char &nBlue,
                          unsigned char &nAlpha) const {
    if (mBpp == kBpp32 || mBpp <= kMaxPaletteBpp) {
        const unsigned int nWord = ReadWord(pData);
        if ((mOrder & kOrderGs) != 0) {
            nAlpha = static_cast<unsigned char>(((nWord >> 24) * kOpaque) >> 7);
        } else {
            nAlpha = static_cast<unsigned char>(nWord >> 24);
        }
        if ((mOrder & kOrderRGBA) != 0) {
            nBlue = static_cast<unsigned char>(nWord >> 16);
            nGreen = static_cast<unsigned char>(nWord >> 8);
            nRed = static_cast<unsigned char>(nWord);
        } else {
            nRed = static_cast<unsigned char>(nWord >> 16);
            nGreen = static_cast<unsigned char>(nWord >> 8);
            nBlue = static_cast<unsigned char>(nWord);
        }
    } else if (mBpp == kBpp16) {
        const unsigned int nHalf = ReadHalf(pData);
        nAlpha = (nHalf & kAlpha16Bit) != 0 ? kOpaque : 0;
        const auto nHigh = static_cast<unsigned char>((nHalf & kRed16Mask) >> 7);
        const auto nMiddle = static_cast<unsigned char>((nHalf & kGreen16Mask) >> 2);
        const auto nLow = static_cast<unsigned char>(nHalf << 3);
        if ((mOrder & kOrderRGBA) != 0) {
            nBlue = nHigh;
            nGreen = nMiddle;
            nRed = nLow;
        } else {
            nRed = nHigh;
            nGreen = nMiddle;
            nBlue = nLow;
        }
    } else {
        nAlpha = kOpaque;
        if ((mOrder & kOrderRGBA) != 0) {
            nBlue = pData[2];
            nGreen = pData[1];
            nRed = pData[0];
        } else {
            nRed = pData[2];
            nGreen = pData[1];
            nBlue = pData[0];
        }
    }
}

void RndBitmap::WriteColor(unsigned char nRed,
                           unsigned char nGreen,
                           unsigned char nBlue,
                           unsigned char nAlpha,
                           unsigned char *pData) const {
    if (mBpp == kBpp32 || mBpp <= kMaxPaletteBpp) {
        unsigned int nWord;
        if ((mOrder & kOrderRGBA) == 0) {
            // Yes, the binary halves the alpha only for the RGBA order.
            nWord = (nAlpha << 24) | (nRed << 16) | (nGreen << 8) | nBlue;
        } else if ((mOrder & kOrderGs) != 0) {
            nWord = (((nAlpha + 1) >> 1) << 24) | (nBlue << 16) | (nGreen << 8) | nRed;
        } else {
            nWord = (nAlpha << 24) | (nBlue << 16) | (nGreen << 8) | nRed;
        }
        std::memcpy(pData, &nWord, sizeof(nWord));
    } else if (mBpp == kBpp16) {
        const unsigned int nHigh = (mOrder & kOrderRGBA) != 0 ? nBlue : nRed;
        const unsigned int nLow = (mOrder & kOrderRGBA) != 0 ? nRed : nBlue;
        const auto nHalf = static_cast<unsigned short>(
            ((nAlpha & kAlpha8Bit) << 8) | ((nHigh & kComponent16Mask) << 7) |
            ((nGreen & kComponent16Mask) << 2) | (nLow >> 3));
        std::memcpy(pData, &nHalf, sizeof(nHalf));
    } else if ((mOrder & kOrderRGBA) != 0) {
        pData[2] = nBlue;
        pData[0] = nRed;
        pData[1] = nGreen;
    } else {
        pData[2] = nRed;
        pData[0] = nBlue;
        pData[1] = nGreen;
    }
}

void RndBitmap::SetMip(RndBitmap *pMip) {
    if (pMip == nullptr) {
        return;
    }
    if (mBpp != pMip->mBpp || !SamePalette(*pMip)) {
        DebugNotify("Mipmap has incorrect palette");
        return;
    }
    if (pMip->mWidth != mWidth >> 1 || pMip->mHeight != mHeight >> 1) {
        DebugNotify("Mipmap has incorrect dimensions");
        return;
    }
    delete mMip;
    mMip = pMip;
    pMip->mPalette = mPalette;
}

void RndBitmap::Reset() {
    mBpp = kBpp32;
    mOrder = kOrderRGBA;
    mRowBytes = 0;
    mHeight = 0;
    mWidth = 0;
    mPalette = nullptr;
    mPixels = nullptr;
    if (mBuffer != nullptr) {
        PoolMemFree(mBuffer);
        mBuffer = nullptr;
    }
    delete mMip;
    mMip = nullptr;
}

void RndBitmap::Create(const RndBitmap &source, int nBpp, int nOrder) {
    const RndBitmap *pSource = &source;
    RndBitmap *pDest = this;
    while (pSource != nullptr) {
        pDest->Create(pSource->mWidth, pSource->mHeight, 0, nBpp, nOrder);
        if (pDest == this) {
            const int nEntries = source.mPalette != nullptr ? 1 << source.mBpp : 0;
            for (int i = 0; i < nEntries; ++i) {
                unsigned char nRed;
                unsigned char nGreen;
                unsigned char nBlue;
                unsigned char nAlpha;
                source.PaletteColor(i, nRed, nGreen, nBlue, nAlpha);
                SetPaletteColor(i, nRed, nGreen, nBlue, nAlpha);
            }
        } else {
            pDest->mPalette = mPalette;
        }
        pDest->Blit(*pSource, 0, 0, 0, 0, pSource->mWidth, pSource->mHeight);
        pSource = pSource->mMip;
        if (pSource == nullptr) {
            break;
        }
        delete pDest->mMip;
        pDest->mMip = nullptr;
        auto *pMip = new RndBitmap;
        pDest->mMip = pMip;
        pDest = pMip;
    }
}

bool RndBitmap::CheckDims(int nWidth, int nHeight, int nBpp) {
    if (nWidth <= 0 || nHeight <= 0) {
        DebugNotify("Width or height below 1");
        return false;
    }
    if (nBpp != kBpp4 && nBpp != kBpp8 && nBpp != kBpp16 && nBpp != kBpp24 && nBpp != kBpp32) {
        DebugNotify("Invalid bpp %d", nBpp);
        return false;
    }
    return true;
}

void RndBitmap::Create(int nWidth, int nHeight, int nRowBytes, int nBpp, int nOrder) {
    Create(nWidth, nHeight, nRowBytes, nBpp, nOrder, nullptr, nullptr, nullptr);
    const int nPaletteBytes = PaletteBytes();
    const int nPixelBytes = PixelBytes();
    auto *pBuffer =
        static_cast<unsigned char *>(PoolMemAlloc(nPaletteBytes + nPixelBytes, "Bitmap buf", 0));
    mPixels = pBuffer + nPaletteBytes;
    mPalette = nPaletteBytes != 0 ? pBuffer : nullptr;
    mBuffer = pBuffer;
}

void RndBitmap::Create(int nWidth,
                       int nHeight,
                       int nRowBytes,
                       int nBpp,
                       int nOrder,
                       unsigned char *pPalette,
                       unsigned char *pPixels,
                       unsigned char *pBuffer) {
    if (!CheckDims(nWidth, nHeight, nBpp)) {
        return;
    }
    mWidth = static_cast<unsigned short>(nWidth);
    mHeight = static_cast<unsigned short>(nHeight);
    mRowBytes = static_cast<unsigned short>(nRowBytes);
    mOrder = static_cast<unsigned char>(nOrder);
    mPixels = pPixels;
    mBpp = static_cast<unsigned char>(nBpp);
    mPalette = pPalette;
    if (pPalette != nullptr && mBpp > kMaxPaletteBpp) {
        DebugNotify("Palette not allowed for bpp %d", mBpp);
        mPalette = nullptr;
    }
    NormalizeLayout();
    if (mBuffer != nullptr) {
        PoolMemFree(mBuffer);
        mBuffer = nullptr;
    }
    mBuffer = pBuffer;
}

void RndBitmap::Create(unsigned char *pBuffer) {
    if (pBuffer == nullptr) {
        DebugNotify("Load buffer is empty");
        return;
    }
    unsigned char *pData = pBuffer + sizeof(Header);
    if ((reinterpret_cast<std::uintptr_t>(pData) & (kLoadAlignment - 1)) != 0) {
        DebugNotify("Load buffer isn't aligned to 16 bytes");
        return;
    }
    Header header;
    std::memcpy(&header, pBuffer, sizeof(header));
    Create(header.mWidth, header.mHeight, 0, header.mBpp, header.mOrder, nullptr, nullptr, pBuffer);
    const int nPaletteBytes = PaletteBytes();
    mPalette = nPaletteBytes != 0 ? pData : nullptr;
    pData += nPaletteBytes;
    int nPixelBytes = PixelBytes();
    mPixels = pData;
    pData += nPixelBytes;
    RndBitmap *pLast = this;
    while (header.mNumMips-- != 0) {
        delete pLast->mMip;
        pLast->mMip = nullptr;
        nPixelBytes >>= 2;
        auto *pMip = new RndBitmap;
        pLast->mMip = pMip;
        pLast = pMip;
        header.mWidth >>= 1;
        header.mHeight >>= 1;
        pMip->Create(header.mWidth, header.mHeight, 0, mBpp, mOrder, mPalette, pData, nullptr);
        pData += nPixelBytes;
    }
}

int RndBitmap::PixelOffset(int nX, int nY, int &nHighNibble) const {
    if ((mOrder & kOrderSwizzled) == 0) {
        nHighNibble = nX & 1;
        return nY * mRowBytes + ((nX * mBpp) >> 3);
    }
    if (mBpp == kBpp8) {
        const int nBlockRow = nY >> 2;
        const int nPageBytes = mRowBytes * 2;
        const int nBase = nBlockRow * 2 * nPageBytes + ((nX >> 4) << 5);
        const unsigned char *pTable = (nBlockRow & 1) != 0 ? g_abGsSwizzle8Odd : g_abGsSwizzle8Even;
        int nOffset = pTable[nX % 16 + (nY % 4) * 16];
        if (nOffset >= 32) {
            nOffset += nPageBytes - 32;
        }
        return nBase + nOffset;
    }
    const int nQuad = (nY >> 2) % 4;
    int nColumn;
    int nRow;
    int nPageBytes;
    if (mWidth > kSwizzle4PageSize && mHeight > kSwizzle4PageSize) {
        // Yes, the binary takes the column from the row and the row from the column here.
        nColumn = ((nY >> 7) << 5) + (((nX % kSwizzle4PageSize) >> 5) << 3) + nQuad * 2;
        nRow = ((nX >> 7) << 6) + (((nY % kSwizzle4PageSize) >> 4) << 3);
        nPageBytes = ((mWidth >> 7) << 8) + ((mHeight * 2) & 0xe0);
    } else {
        nColumn = ((nX >> 5) << 3) + nQuad * 2;
        nRow = (nY >> 4) << 3;
        nPageBytes = mHeight * 2;
    }
    const int nBase = nColumn * nPageBytes + nRow * 4;
    const unsigned char *pTable = (nQuad & 1) != 0 ? g_abGsSwizzle4Odd : g_abGsSwizzle4Even;
    int nOffset = pTable[nX % 32 + (nY % 4) * 32];
    nHighNibble = nOffset & 1;
    nOffset >>= 1;
    if (nOffset >= 32) {
        nOffset += nPageBytes - 32;
    }
    return nBase + nOffset;
}

int RndBitmap::PixelIndex(int nX, int nY) const {
    int nHighNibble;
    const unsigned char *pPixel = mPixels + PixelOffset(nX, nY, nHighNibble);
    if (mBpp == kBpp8) {
        return *pPixel;
    }
    return nHighNibble != 0 ? *pPixel >> 4 : *pPixel & 0xf;
}

void RndBitmap::SetPixelIndex(int nX, int nY, unsigned char nIndex) {
    int nHighNibble;
    unsigned char *pPixel = mPixels + PixelOffset(nX, nY, nHighNibble);
    if (mBpp == kBpp8) {
        *pPixel = nIndex;
    } else if (nHighNibble != 0) {
        *pPixel = static_cast<unsigned char>((nIndex << 4) | (*pPixel & 0xf));
    } else {
        *pPixel = static_cast<unsigned char>(nIndex | (*pPixel & 0xf0));
    }
}

int RndBitmap::MaxPixelIndex() const {
    if (mBpp > kMaxPaletteBpp) {
        return -1;
    }
    int nMax = -1;
    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            const int nIndex = PixelIndex(x, y);
            if (nMax < nIndex) {
                nMax = nIndex;
            }
        }
    }
    return nMax;
}

void RndBitmap::ConvertTo8Bpp() {
    RndBitmap converted;
    converted.Create(mWidth, mHeight, 0, kBpp8, mOrder);
    std::memcpy(converted.mPalette, mPalette, PaletteBytes());
    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            converted.SetPixelIndex(x, y, static_cast<unsigned char>(PixelIndex(x, y)));
        }
    }
    if (mBuffer != nullptr) {
        PoolMemFree(mBuffer);
        mBuffer = nullptr;
    }
    mPalette = converted.mPalette;
    mRowBytes = converted.mRowBytes;
    mPixels = converted.mPixels;
    mBuffer = converted.mBuffer;
    mBpp = converted.mBpp;
    converted.mBuffer = nullptr;
}

bool RndBitmap::LoadMip(const char *pszPath) {
    RndBitmap *pLast = this;
    while (pLast->mMip != nullptr) {
        pLast = pLast->mMip;
    }
    auto *pMip = new RndBitmap;
    if (pMip->LoadBmp(pszPath)) {
        pLast->SetMip(pMip);
        return true;
    }
    delete pMip;
    return false;
}

void RndBitmap::LoadAlpha(const char *pszPath) {
    RndBitmap alpha;
    if (!alpha.LoadBmp(pszPath)) {
        return;
    }
    if (alpha.mWidth != mWidth || alpha.mHeight != mHeight ||
        (alpha.mPalette == nullptr) != (mPalette == nullptr)) {
        DebugNotify("%s: alpha pair doesn't match size or palettization", pszPath);
        return;
    }
    unsigned char nUnused;
    if (mBpp > kMaxPaletteBpp) {
        for (int y = 0; y < mHeight; ++y) {
            for (int x = 0; x < mWidth; ++x) {
                unsigned char nRed;
                unsigned char nGreen;
                unsigned char nBlue;
                unsigned char nAlpha;
                PixelColor(x, y, nRed, nGreen, nBlue, nUnused);
                alpha.PixelColor(x, y, nAlpha, nUnused, nUnused, nUnused);
                SetPixelColor(x, y, nRed, nGreen, nBlue, nAlpha);
            }
        }
        return;
    }
    const int nColors = MaxPixelIndex() + 1;
    const int nAlphas = alpha.MaxPixelIndex() + 1;
    if ((1 << mBpp) < nColors * nAlphas) {
        if (nColors * nAlphas <= 1 << kBpp8) {
            ConvertTo8Bpp();
        } else {
            DebugWarn("%s: alpha combination has too many colors", pszPath);
        }
    }
    for (int j = 0; j < nAlphas; ++j) {
        unsigned char nAlpha;
        alpha.PaletteColor(j, nAlpha, nUnused, nUnused, nUnused);
        for (int i = 0; i < nColors; ++i) {
            unsigned char nRed;
            unsigned char nGreen;
            unsigned char nBlue;
            PaletteColor(i, nRed, nGreen, nBlue, nUnused);
            SetPaletteColor(j * nColors + i, nRed, nGreen, nBlue, nAlpha);
        }
    }
    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            const int nColor = PixelIndex(x, y);
            const int nIndex = alpha.PixelIndex(x, y) * nColors + nColor;
            SetPixelIndex(x, y, static_cast<unsigned char>(nIndex));
        }
    }
}

bool RndBitmap::LoadBmp(const char *pszPath) {
    File *pFile = File::New(pszPath, kFileModeRead, 0);
    if (pFile == nullptr) {
        return false;
    }
    unsigned short nSignature;
    pFile->Read(&nSignature, sizeof(nSignature));
    if (nSignature != kBmpSignature) {
        DebugNotify("%s not BMP format", pszPath);
        delete pFile;
        return false;
    }
    BmpFileHeader fileHeader;
    pFile->Read(&fileHeader, sizeof(fileHeader));
    BmpInfoHeader info;
    pFile->Read(&info, sizeof(info));
    if (info.mBitCount < kBmpMinBitCount) {
        DebugNotify("%s: Unsupported bit depth %d", pszPath, info.mBitCount);
        delete pFile;
        return false;
    }
    if (info.mCompression != 0) {
        DebugNotify("%s: Unsupported compression %d", pszPath, info.mCompression);
        delete pFile;
        return false;
    }
    const int nBitCount = info.mBitCount;
    const int nPaletteBytes = nBitCount <= kMaxPaletteBpp ? kPaletteEntryBytes << nBitCount : 0;
    int nRowBits;
    if (nBitCount == kBpp4 && (info.mWidth & 1) != 0) {
        nRowBits = info.mWidth * nBitCount + nBitCount;
    } else {
        nRowBits = info.mWidth * nBitCount;
    }
    const int nRowBytes =
        (((nRowBits >> 3) + kBmpRowAlignment - 1) / kBmpRowAlignment) * kBmpRowAlignment;
    const int nPixelBytes = nRowBytes * info.mHeight;
    auto *pBuffer =
        static_cast<unsigned char *>(PoolMemAlloc(nPaletteBytes + nPixelBytes, "Bitmap buf", 0));
    unsigned char *pPalette = nullptr;
    if (nPaletteBytes != 0) {
        pPalette = pBuffer;
        if (info.mClrUsed == 0 || (1u << nBitCount) < info.mClrUsed) {
            pFile->Read(pPalette, nPaletteBytes);
        } else {
            std::memset(pPalette, 0, nPaletteBytes);
            pFile->Read(pPalette, info.mClrUsed * kPaletteEntryBytes);
        }
    }
    unsigned char *pPixels = pBuffer + nPaletteBytes;
    pFile->Seek(fileHeader.mOffBits, 0);
    if (info.mHeight < 0) {
        pFile->Read(pPixels, nPixelBytes);
    } else {
        for (int nRow = info.mHeight - 1; nRow >= 0; --nRow) {
            pFile->Read(pPixels + nRow * nRowBytes, nRowBytes);
        }
    }
    if (nBitCount == kBpp4) {
        for (unsigned char *pPixel = pPixels; pPixel != pPixels + nPixelBytes; ++pPixel) {
            *pPixel = static_cast<unsigned char>((*pPixel >> 4) | ((*pPixel & 0xf) << 4));
        }
    }
    for (int i = nPaletteBytes - kPaletteEntryBytes; i >= 0; i -= kPaletteEntryBytes) {
        pPalette[i + 3] = kOpaque;
    }
    if (nBitCount == kBpp16) {
        for (int i = nPixelBytes - 2; i >= 0; i -= 2) {
            pPixels[i + 1] |= kAlpha8Bit;
        }
    }
    Create(info.mWidth, info.mHeight, nRowBytes, nBitCount, 0, pPalette, pPixels, pBuffer);
    delete pFile;
    int nMode = kBmpAlphaNone;
    if (std::strstr(pszPath, "_tb") != nullptr) {
        nMode = kBmpAlphaTransparentBlack;
    } else if (std::strstr(pszPath, "_gw") != nullptr) {
        nMode = kBmpAlphaGrayWhite;
    } else if (std::strstr(pszPath, "_ga") != nullptr) {
        nMode = kBmpAlphaGrayAlpha;
    }
    if (nMode == kBmpAlphaNone) {
        return true;
    }
    if (mBpp <= kMaxPaletteBpp) {
        for (int i = (mPalette != nullptr ? 1 << mBpp : 0) - 1; i >= 0; --i) {
            unsigned char nRed;
            unsigned char nGreen;
            unsigned char nBlue;
            unsigned char nAlpha;
            PaletteColor(i, nRed, nGreen, nBlue, nAlpha);
            ApplyBmpAlphaMode(nMode, nRed, nGreen, nBlue, nAlpha);
            SetPaletteColor(i, nRed, nGreen, nBlue, nAlpha);
        }
        return true;
    }
    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            unsigned char nRed;
            unsigned char nGreen;
            unsigned char nBlue;
            unsigned char nAlpha;
            PixelColor(x, y, nRed, nGreen, nBlue, nAlpha);
            ApplyBmpAlphaMode(nMode, nRed, nGreen, nBlue, nAlpha);
            SetPixelColor(x, y, nRed, nGreen, nBlue, nAlpha);
        }
    }
    return true;
}

void RndBitmap::SaveBmp(const char *pszPath) const {
    if ((mOrder & kOrderRGBA) != 0) {
        DebugNotify("Order isn't kARGB");
        return;
    }
    File *pFile = File::New(pszPath, kFileModeWrite, 0);
    const unsigned short nSignature = kBmpSignature;
    pFile->Write(&nSignature, sizeof(nSignature));
    BmpFileHeader fileHeader;
    fileHeader.mOffBits =
        PaletteBytes() + sizeof(nSignature) + sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
    fileHeader.mReserved1 = 0;
    fileHeader.mReserved2 = 0;
    fileHeader.mSize = fileHeader.mOffBits + PixelBytes();
    pFile->Write(&fileHeader, sizeof(fileHeader));
    BmpInfoHeader info;
    info.mSize = sizeof(BmpInfoHeader);
    info.mWidth = mWidth;
    info.mHeight = mHeight;
    info.mPlanes = 1;
    info.mBitCount = mBpp;
    info.mYPelsPerMeter = kBmpPelsPerMeter;
    info.mCompression = 0;
    info.mSizeImage = 0;
    info.mXPelsPerMeter = kBmpPelsPerMeter;
    info.mClrUsed = 0;
    info.mClrImportant = 0;
    pFile->Write(&info, sizeof(info));
    if (mPalette != nullptr) {
        pFile->Write(mPalette, kPaletteEntryBytes << mBpp);
    }
    for (int nRow = mHeight - 1; nRow >= 0; --nRow) {
        const unsigned char *pRow = mPixels + mRowBytes * nRow;
        if (mBpp == kBpp4) {
            for (const unsigned char *pPixel = pRow; pPixel != pRow + mRowBytes; ++pPixel) {
                const auto nSwapped =
                    static_cast<unsigned char>((*pPixel >> 4) | ((*pPixel & 0xf) << 4));
                pFile->Write(&nSwapped, sizeof(nSwapped));
            }
        } else {
            pFile->Write(pRow, mRowBytes);
        }
    }
    delete pFile;
}

bool RndBitmap::SamePalette(const RndBitmap &other) const {
    if (mPalette == other.mPalette) {
        return true;
    }
    for (int i = (1 << mBpp) - 1; i >= 0; --i) {
        unsigned char color[kPaletteEntryBytes];
        unsigned char otherColor[kPaletteEntryBytes];
        PaletteColor(i, color[0], color[1], color[2], color[3]);
        other.PaletteColor(i, otherColor[0], otherColor[1], otherColor[2], otherColor[3]);
        if (std::memcmp(color, otherColor, sizeof(color)) != 0) {
            return false;
        }
    }
    return true;
}

bool RndBitmap::SameFormat(const RndBitmap &other) const {
    if (mBpp != other.mBpp) {
        return false;
    }
    if (((mOrder | other.mOrder) & kOrderSwizzled) != 0) {
        return false;
    }
    if (mBpp <= kMaxPaletteBpp) {
        return SamePalette(other);
    }
    return mOrder == other.mOrder;
}

void RndBitmap::Blit(const RndBitmap &source,
                     int nDestX,
                     int nDestY,
                     int nSourceX,
                     int nSourceY,
                     int nWidth,
                     int nHeight) {
    if (SameFormat(source)) {
        const int nBytes = (nWidth * mBpp) >> 3;
        // Yes, the binary swaps the corners of the source and the destination in this copy.
        for (; nHeight > 0; --nHeight) {
            unsigned char *pDest = mPixels + mRowBytes * nSourceY + ((mBpp * nSourceX) >> 3);
            const unsigned char *pSource =
                source.mPixels + source.mRowBytes * nDestY + ((source.mBpp * nDestX) >> 3);
            std::memcpy(pDest, pSource, nBytes);
            ++nDestY;
            ++nSourceY;
        }
        return;
    }
    if (mPalette != nullptr && source.mPalette != nullptr) {
        unsigned char map[1 << kBpp8];
        for (int i = (1 << source.mBpp) - 1; i >= 0; --i) {
            unsigned char nRed;
            unsigned char nGreen;
            unsigned char nBlue;
            unsigned char nAlpha;
            source.PaletteColor(i, nRed, nGreen, nBlue, nAlpha);
            map[i] = NearestColor(nRed, nGreen, nBlue, nAlpha);
        }
        for (; nHeight > 0; --nHeight) {
            for (int i = 0; i < nWidth; ++i) {
                SetPixelIndex(nDestX + i, nDestY, map[source.PixelIndex(nSourceX + i, nSourceY)]);
            }
            ++nDestY;
            ++nSourceY;
        }
        return;
    }
    for (; nHeight > 0; --nHeight) {
        for (int i = 0; i < nWidth; ++i) {
            unsigned char nRed;
            unsigned char nGreen;
            unsigned char nBlue;
            unsigned char nAlpha;
            source.PixelColor(nSourceX + i, nSourceY, nRed, nGreen, nBlue, nAlpha);
            SetPixelColor(nDestX + i, nDestY, nRed, nGreen, nBlue, nAlpha);
        }
        ++nDestY;
        ++nSourceY;
    }
}

void RndBitmap::PixelColor(int nX,
                           int nY,
                           unsigned char &nRed,
                           unsigned char &nGreen,
                           unsigned char &nBlue,
                           unsigned char &nAlpha) const {
    if (mBpp > kMaxPaletteBpp) {
        ReadColor(mPixels + nY * mRowBytes + ((nX * mBpp) >> 3), nRed, nGreen, nBlue, nAlpha);
    } else {
        PaletteColor(PixelIndex(nX, nY), nRed, nGreen, nBlue, nAlpha);
    }
}

void RndBitmap::SetPixelColor(int nX,
                              int nY,
                              unsigned char nRed,
                              unsigned char nGreen,
                              unsigned char nBlue,
                              unsigned char nAlpha) {
    if (mBpp > kMaxPaletteBpp) {
        WriteColor(nRed, nGreen, nBlue, nAlpha, mPixels + nY * mRowBytes + ((nX * mBpp) >> 3));
    } else {
        SetPixelIndex(nX, nY, NearestColor(nRed, nGreen, nBlue, nAlpha));
    }
}

int RndBitmap::PaletteOffset(int nIndex) const {
    if (mBpp == kBpp8 && (mOrder & kOrderGs) != 0) {
        const int nBits = nIndex & kGsPaletteSwapMask;
        if (nBits == kGsPaletteSwapLow) {
            nIndex += kGsPaletteSwapLow;
        } else if (nBits == kGsPaletteSwapHigh) {
            nIndex -= kGsPaletteSwapLow;
        }
    }
    return nIndex;
}

void RndBitmap::PaletteColor(int nIndex,
                             unsigned char &nRed,
                             unsigned char &nGreen,
                             unsigned char &nBlue,
                             unsigned char &nAlpha) const {
    ReadColor(mPalette + PaletteOffset(nIndex) * kPaletteEntryBytes, nRed, nGreen, nBlue, nAlpha);
}

void RndBitmap::SetPaletteColor(int nIndex,
                                unsigned char nRed,
                                unsigned char nGreen,
                                unsigned char nBlue,
                                unsigned char nAlpha) {
    WriteColor(nRed, nGreen, nBlue, nAlpha, mPalette + PaletteOffset(nIndex) * kPaletteEntryBytes);
}

void RndBitmap::Print(PrnStream &stream) const {
    stream << "\n\twidth:" << static_cast<unsigned int>(mWidth);
    stream << "\n\theight:" << static_cast<unsigned int>(mHeight);
    stream << "\n\trowBytes:" << static_cast<unsigned int>(mRowBytes);
    stream << "\n\tbpp:" << mBpp;
    stream << "\n\torder:" << mOrder;
    stream << "\n\tnumMips:" << NumMips();
}

void RndBitmap::Save(BinStream &stream) const {
    Header header;
    std::memset(&header, 0, sizeof(header));
    header.mHeight = mHeight;
    header.mRowBytes = mRowBytes;
    header.mBpp = mBpp;
    header.mOrder = mOrder;
    header.mRev = sRev;
    header.mWidth = mWidth;
    header.mNumMips = static_cast<unsigned char>(NumMips());
    stream.Write(&header, sizeof(header));
    if (mPalette != nullptr) {
        stream.Write(mPalette, PaletteBytes());
    }
    for (const RndBitmap *pBitmap = this; pBitmap != nullptr; pBitmap = pBitmap->mMip) {
        stream.Write(pBitmap->mPixels, pBitmap->PixelBytes());
    }
}

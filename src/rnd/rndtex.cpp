#include "rnd/rndtex.h"

#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

constexpr int kDefaultMipMapK = -128;
constexpr int kDefaultBpp = 32;

// The limits of a texture dimension and of the bitmap size.
constexpr int kMinDim = 8;
constexpr int kMaxDim = 1024;
constexpr int kMaxBytes = 524272;
constexpr int kMaxPackedBpp = 16;

// The longest file path Load() reads.
constexpr int kMaxPath = 256;

// The first version with each later field, and the versions with legacy fields.
constexpr int kRevShortSize = 1;
constexpr int kRevUnusedByteFirst = 1;
constexpr int kRevUnusedByteLast = 2;
constexpr int kRevMipMapK = 4;
constexpr int kRevRendered = 5;

// The bits of the flag word before kRevRendered.
constexpr int kFlagTransparentWhite = 1;
constexpr int kFlagTransparentBlack = 2;
constexpr int kFlagGrayAlpha = 0x10;
constexpr int kFlagGrayWhite = 0x20;
constexpr int kFlagCubeMap = 0x40;

// Insert a suffix before the extension of a path.
void InsertSuffix(FilePath &file, const char *pszSuffix) {
    const int nPos = file.Find('.');
    const String suffix(pszSuffix);
    file.Insert(nPos, suffix);
}

} // namespace

const char *RndTex::sClassName = "Tex";
int RndTex::sRev = 5;

RndTex::RndTex(const char *pszName) : RndObject(pszName) {
    mBpp = kDefaultBpp;
    mMipMapK = kDefaultMipMapK;
    mRendered = 0;
    mWidth = 0;
    mHeight = 0;
}

RndTex::~RndTex() {
    ResetBitmap();
}

void RndTex::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndTex]\n";
    stream << "width:" << mWidth << " height:" << mHeight << " bpp:" << mBpp
           << " mipMapK:" << mMipMapK << " file:" << mFile.RelativePath()
           << " rendered:" << (mRendered != 0) << "\n";
    if (stream.mDumpLevel < 2) {
        return;
    }
    stream << "bitmap:";
    mBitmap.Print(stream);
    stream << "\n";
}

void RndTex::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    stream.WriteEndian(&mWidth, sizeof(mWidth));
    stream.WriteEndian(&mHeight, sizeof(mHeight));
    stream.WriteEndian(&mBpp, sizeof(mBpp));
    stream.WriteString(mFile.RelativePath());
    stream.WriteEndian(&mMipMapK, sizeof(mMipMapK));
    const char nRendered = static_cast<char>(mRendered);
    stream.Write(&nRendered, sizeof(nRendered));
}

void RndTex::Copy(const RndObject *pSource, [[maybe_unused]] int nFlags) {
    const RndTex *pTex = pSource != nullptr ? dynamic_cast<const RndTex *>(pSource) : nullptr;
    ResetBitmap();
    mWidth = pTex->mWidth;
    mHeight = pTex->mHeight;
    mBpp = pTex->mBpp;
    mFile = pTex->mFile;
    mMipMapK = pTex->mMipMapK;
    mRendered = pTex->mRendered;
    SyncBitmap();
}

void RndTex::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new Tex");
        return;
    }
    ResetBitmap();
    if (nRev == kRevShortSize) {
        short nWidth;
        stream.ReadEndian(&nWidth, sizeof(nWidth));
        short nHeight;
        stream.ReadEndian(&nHeight, sizeof(nHeight));
        mWidth = nWidth;
        mHeight = nHeight;
    } else {
        stream.ReadEndian(&mWidth, sizeof(mWidth));
        stream.ReadEndian(&mHeight, sizeof(mHeight));
    }
    stream.ReadEndian(&mBpp, sizeof(mBpp));
    char szFile[kMaxPath];
    stream.ReadString(szFile, sizeof(szFile));
    mFile.Set(szFile);
    if (nRev < kRevRendered) {
        int nFlags;
        stream.ReadEndian(&nFlags, sizeof(nFlags));
        if (nFlags != 0 && mFile.mLength != 0) {
            if ((nFlags & kFlagTransparentWhite) != 0) {
                DebugNotify("%s: kTransparentWhite no longer supported", mName.c_str());
            } else if ((nFlags & kFlagTransparentBlack) != 0) {
                InsertSuffix(mFile, "_tb");
            } else if ((nFlags & kFlagGrayAlpha) != 0) {
                InsertSuffix(mFile, "_ga");
            } else if ((nFlags & kFlagGrayWhite) != 0) {
                InsertSuffix(mFile, "_gw");
            } else if ((nFlags & kFlagCubeMap) != 0) {
                DebugNotify("%s: kCubeMap no longer supported", mName.c_str());
            }
        }
    }
    if (nRev >= kRevUnusedByteFirst && nRev <= kRevUnusedByteLast) {
        unsigned char nUnused;
        stream.Read(&nUnused, sizeof(nUnused));
    }
    if (nRev >= kRevMipMapK) {
        stream.ReadEndian(&mMipMapK, sizeof(mMipMapK));
    }
    if (nRev >= kRevRendered) {
        unsigned char nRendered;
        stream.Read(&nRendered, sizeof(nRendered));
        mRendered = nRendered != 0;
    }
    SyncBitmap();
}

void RndTex::SyncBitmap() {
    void *pData = nullptr;
    int nSize;
    TheManager.GetResource(mFile, &pData, &nSize);
    if (pData != nullptr) {
        mBitmap.Create(static_cast<unsigned char *>(pData));
        mWidth = mBitmap.mWidth;
        mHeight = mBitmap.mHeight;
        mBpp = mBitmap.mBpp;
    }
    if (!CheckSize()) {
        mBitmap.Reset();
    } else if (pData == nullptr) {
        mBitmap.Create(mWidth, mHeight, 0, mBpp, RndBitmap::kOrderRGBA | RndBitmap::kOrderGs);
    }
}

void RndTex::ResetBitmap() {
    mBitmap.Reset();
}

void RndTex::SetBitmap(
    int nWidth, int nHeight, int nBpp, const FilePath &file, int nMipMapK, int nRendered) {
    ResetBitmap();
    mWidth = nWidth;
    mHeight = nHeight;
    mBpp = nBpp;
    mFile = file;
    mMipMapK = nMipMapK;
    mRendered = nRendered;
    SyncBitmap();
}

const char *RndTex::CheckDim(int nSize) {
    bool bPowerOfTwo = false;
    if (nSize > 0) {
        int n = nSize;
        while ((n & 1) == 0) {
            n >>= 1;
        }
        bPowerOfTwo = n == 1;
    }
    const char *pszError = nullptr;
    if (!bPowerOfTwo) {
        pszError = "%s: dimensions not power of 2";
    }
    if (nSize < kMinDim) {
        pszError = "%s: dimensions less than 8";
    }
    if (nSize > kMaxDim) {
        pszError = "%s: dimensions greater than 1024";
    }
    return pszError;
}

bool RndTex::CheckSize() {
    const char *pszError = CheckDim(mWidth);
    if (pszError == nullptr) {
        pszError = CheckDim(mHeight);
    }
    if (pszError == nullptr) {
        const int nBpp = mBpp > kMaxPackedBpp ? kDefaultBpp : mBpp;
        if (kMaxBytes < (mWidth * mHeight * nBpp) >> 3) {
            pszError = "%s: size over 524,272 bytes";
        }
    }
    if (pszError != nullptr && mWidth != 0 && mHeight != 0) {
        DebugNotify(pszError, mName.c_str());
    }
    return pszError == nullptr;
}

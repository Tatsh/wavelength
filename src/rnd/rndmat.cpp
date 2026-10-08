#include "rnd/rndmat.h"

#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

// The first version that stores the blend modes themselves rather than source and destination
// factors, and the versions that changed the colours and the flags.
constexpr int kRevBlendMode = 3;
constexpr int kRevNormalize = 4;
constexpr int kRevFlat = 5;
constexpr int kRevSeparateAlphas = 6;
constexpr int kRevMultiPassCount = 7;
constexpr int kRevEdgeAlpha = 8;
constexpr int kRevBaseAmbient = 9;
constexpr int kRevMultiPassFlag = 2;

// A version before kRevBlendMode stores a surface blend as a source and a destination code. A pair
// the table does not list leaves the blend unchanged.
struct OldBlend {
    int mSrc;
    int mDest;
    int mBlend;
};

constexpr OldBlend kOldBlends[] = {
    {0, 1, RndMat::kBlendDest},
    {1, 0, RndMat::kBlendSrc},
    {1, 1, RndMat::kBlendAdd},
    {2, 1, RndMat::kBlendSrcAdd},
    {4, 1, RndMat::kBlendSrcAlphaAdd},
    {6, 0, RndMat::kBlendMultiply},
    {6, 2, RndMat::kBlendMultiply2},
    {4, 5, RndMat::kBlendSrcAlpha},
    {5, 4, RndMat::kBlendInvSrcAlpha},
    {8, 9, RndMat::kBlendDestAlpha},
    {9, 8, RndMat::kBlendInvDestAlpha},
};

// A version before kRevBlendMode stores a stage blend as a source code, the index here, and a
// destination code nothing reads. A code past the table leaves the blend unchanged.
constexpr int kOldStageBlends[] = {
    RndMat::kBlendDest,
    RndMat::kBlendSrc,
    RndMat::kBlendMultiply,
    RndMat::kBlendAdd,
    RndMat::kBlendDestAlpha,
    RndMat::kBlendSrcAlpha,
    RndMat::kBlendInvDestAlpha,
};

void ReadFloat(BinStream &stream, float &fValue) {
    stream.ReadEndian(&fValue, sizeof(fValue));
}

void WriteFloat(BinStream &stream, float fValue) {
    stream.WriteEndian(&fValue, sizeof(fValue));
}

void ReadColor(BinStream &stream, Color &color) {
    ReadFloat(stream, color.r);
    ReadFloat(stream, color.g);
    ReadFloat(stream, color.b);
    ReadFloat(stream, color.a);
}

void WriteColor(BinStream &stream, const Color &color) {
    WriteFloat(stream, color.r);
    WriteFloat(stream, color.g);
    WriteFloat(stream, color.b);
    WriteFloat(stream, color.a);
}

int ReadFlag(BinStream &stream) {
    unsigned char nFlag;
    stream.Read(&nFlag, sizeof(nFlag));
    return nFlag != 0;
}

void WriteFlag(BinStream &stream, int bFlag) {
    const char nFlag = static_cast<char>(bFlag);
    stream.Write(&nFlag, sizeof(nFlag));
}

void ReadRow(BinStream &stream, Vector3 &row) {
    ReadFloat(stream, row.x);
    ReadFloat(stream, row.y);
    ReadFloat(stream, row.z);
}

void WriteRow(BinStream &stream, const Vector3 &row) {
    WriteFloat(stream, row.x);
    WriteFloat(stream, row.y);
    WriteFloat(stream, row.z);
}

void MapOldBlend(int nSrc, int nDest, int &nBlend) {
    for (const OldBlend &entry : kOldBlends) {
        if (entry.mSrc == nSrc && entry.mDest == nDest) {
            nBlend = entry.mBlend;
            return;
        }
    }
}

void MapOldStageBlend(int nSrc, int &nBlend) {
    const auto nIndex = static_cast<unsigned int>(nSrc);
    if (nIndex < sizeof(kOldStageBlends) / sizeof(kOldStageBlends[0])) {
        nBlend = kOldStageBlends[nIndex];
    }
}

// A named object of the registry as a given class, or null for an empty name.
template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.mLength == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

} // namespace

const char *RndMat::sClassName = "Mat";
int RndMat::sRev = 9;
int RndMat::sLoadRev;

RndMat::Stage::Stage() {
    mBlend = kBlendMultiply;
    mWrap = kTexWrapRepeat;
    mCoordIndex = 0;
    mGenMode = kTexGenFixed;
    mUseXfm = 0;
    mTex = nullptr;
    mMat = nullptr;
    mXfm.mBasisX.x = 1.0f;
    mXfm.mBasisX.z = 0.0f;
    mXfm.mBasisX.y = 0.0f;
    mXfm.mBasisY.x = 0.0f;
    mXfm.mBasisY.z = 0.0f;
    mXfm.mBasisY.y = 1.0f;
    mXfm.mBasisZ.x = 0.0f;
    mXfm.mBasisZ.z = 1.0f;
    mXfm.mBasisZ.y = 0.0f;
    mXfm.mTranslation = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
}

void RndMat::Stage::SetTex(RndTex *pTex) {
    if (mTex != nullptr) {
        mTex->RemoveRef(mMat);
    }
    mTex = pTex;
    if (pTex != nullptr) {
        pTex->AddRef(mMat);
    }
}

void RndMat::Stage::Print(PrnStream &stream) const {
    stream << "\n\tblend:" << static_cast<Blend>(mBlend) << " coordIndex:" << mCoordIndex
           << " genMode:" << static_cast<TexGen>(mGenMode) << " xfm:" << mXfm << "\n";
    stream << "useXfm:" << (mUseXfm != 0) << " wrap:" << static_cast<TexWrap>(mWrap)
           << " mat:" << mMat << " tex:" << mTex;
}

void RndMat::Stage::Save(BinStream &stream) const {
    stream.WriteEndian(&mBlend, sizeof(mBlend));
    stream.WriteEndian(&mCoordIndex, sizeof(mCoordIndex));
    stream.WriteEndian(&mGenMode, sizeof(mGenMode));
    WriteRow(stream, mXfm.mBasisX);
    WriteRow(stream, mXfm.mBasisY);
    WriteRow(stream, mXfm.mBasisZ);
    WriteRow(stream, mXfm.mTranslation);
    WriteFlag(stream, mUseXfm);
    stream.WriteEndian(&mWrap, sizeof(mWrap));
    stream.WriteString(mTex != nullptr ? mTex->mName.c_str() : "");
}

void RndMat::Stage::Load(BinStream &stream) {
    if (sLoadRev >= kRevBlendMode) {
        stream.ReadEndian(&mBlend, sizeof(mBlend));
    } else {
        int nSrc;
        int nDest;
        stream.ReadEndian(&nSrc, sizeof(nSrc));
        stream.ReadEndian(&nDest, sizeof(nDest));
        MapOldStageBlend(nSrc, mBlend);
    }
    stream.ReadEndian(&mCoordIndex, sizeof(mCoordIndex));
    int nGenMode;
    stream.ReadEndian(&nGenMode, sizeof(nGenMode));
    mGenMode = nGenMode;
    ReadRow(stream, mXfm.mBasisX);
    ReadRow(stream, mXfm.mBasisY);
    ReadRow(stream, mXfm.mBasisZ);
    ReadRow(stream, mXfm.mTranslation);
    mUseXfm = ReadFlag(stream);
    int nWrap;
    stream.ReadEndian(&nWrap, sizeof(nWrap));
    mWrap = nWrap;
    if (sLoadRev <= 0) {
        mMat = FindByName<RndMat>(stream);
    }
    mTex = FindByName<RndTex>(stream);
}

RndMat::RndMat(const char *pszName) : RndObject(pszName) {
    mBlend = kBlendSrcAlpha;
    mBaseColor.r = 0.0f;
    mBaseColor.a = 1.0f;
    mBaseColor.g = 0.0f;
    mBaseColor.b = 0.0f;
    mLightColor.r = 1.0f;
    mLightColor.a = 1.0f;
    mLightColor.g = 1.0f;
    mLightColor.b = 1.0f;
    mEdgeColor.r = 0.0f;
    mEdgeColor.a = 1.0f;
    mEdgeColor.g = 0.0f;
    mEdgeColor.b = 0.0f;
    mEnable = 1;
    mVertBase = 0;
    mVertLight = 0;
    mVertEdge = 0;
    mNormalize = 0;
    mBaseAmbient = 0;
    mCull = kCullClockwise;
    mFlat = 0;
    mMultiPass = 0;
}

RndMat::~RndMat() {
    ReleaseStageTex();
}

void RndMat::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndMat]\n";
    stream << "stages:" << mStages << "\n";
    stream << "blend:" << static_cast<Blend>(mBlend) << " enable:" << (mEnable != 0)
           << " baseAmbient:" << (mBaseAmbient != 0) << "\n";
    stream << "baseColor:" << mBaseColor << "lightColor:" << mLightColor << "\n";
    stream << "edgeColor:" << mEdgeColor << "vertBase:" << (mVertBase != 0) << "\n";
    stream << "vertLight:" << (mVertLight != 0) << " vertEdge:" << (mVertEdge != 0) << "\n";
    stream << "cull:" << static_cast<CullMode>(mCull) << " multiPass:" << mMultiPass
           << " normalize:" << (mNormalize != 0) << " flat:" << (mFlat != 0) << "\n";
}

void RndMat::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    stream << mStages;
    stream.WriteEndian(&mBlend, sizeof(mBlend));
    WriteColor(stream, mBaseColor);
    WriteColor(stream, mLightColor);
    WriteColor(stream, mEdgeColor);
    WriteFlag(stream, mEnable);
    WriteFlag(stream, mVertBase);
    WriteFlag(stream, mVertLight);
    WriteFlag(stream, mVertEdge);
    stream.WriteEndian(&mCull, sizeof(mCull));
    stream.WriteEndian(&mMultiPass, sizeof(mMultiPass));
    WriteFlag(stream, mNormalize);
    WriteFlag(stream, mFlat);
    WriteFlag(stream, mBaseAmbient);
}

void RndMat::Replace(RndObject *pFrom, RndObject *pTo) {
    for (Stage &stage : mStages) {
        if (stage.mTex != pFrom) {
            continue;
        }
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        if (stage.mTex != nullptr) {
            stage.mTex = pTo != nullptr ? dynamic_cast<RndTex *>(pTo) : nullptr;
        }
        if (stage.mTex != nullptr) {
            stage.mTex->AddRef(this);
        }
    }
}

void RndMat::Copy(const RndObject *pSource, [[maybe_unused]] int nFlags) {
    const RndMat *pMat = pSource != nullptr ? dynamic_cast<const RndMat *>(pSource) : nullptr;
    ReleaseStageTex();
    mBlend = pMat->mBlend;
    mBaseColor = pMat->mBaseColor;
    mLightColor = pMat->mLightColor;
    mEdgeColor = pMat->mEdgeColor;
    mStages = pMat->mStages;
    mMultiPass = pMat->mMultiPass;
    mEnable = pMat->mEnable;
    mVertBase = pMat->mVertBase;
    mVertLight = pMat->mVertLight;
    mVertEdge = pMat->mVertEdge;
    mCull = pMat->mCull;
    mNormalize = pMat->mNormalize;
    mBaseAmbient = pMat->mBaseAmbient;
    Refresh();
}

void RndMat::Load(BinStream &stream) {
    stream.ReadEndian(&sLoadRev, sizeof(sLoadRev));
    if (sLoadRev > sRev) {
        DebugNotify("Can't load new Mat");
        return;
    }
    ReleaseStageTex();
    stream >> mStages;
    if (sLoadRev >= kRevBlendMode) {
        int nBlend;
        stream.ReadEndian(&nBlend, sizeof(nBlend));
        mBlend = nBlend;
    } else {
        int nSrc;
        int nDest;
        stream.ReadEndian(&nSrc, sizeof(nSrc));
        stream.ReadEndian(&nDest, sizeof(nDest));
        MapOldBlend(nSrc, nDest, mBlend);
    }
    ReadColor(stream, mBaseColor);
    ReadColor(stream, mLightColor);
    ReadColor(stream, mEdgeColor);
    Color oldBase{0.0f, 0.0f, 0.0f, 0.0f};
    if (sLoadRev < kRevBaseAmbient) {
        ReadColor(stream, oldBase);
        mBaseAmbient = 1;
        mBaseColor.a = oldBase.a;
    }
    if (sLoadRev < kRevEdgeAlpha) {
        mBaseColor.a = mLightColor.a;
        mEdgeColor.a = 1.0f;
    }
    if (sLoadRev < kRevSeparateAlphas) {
        float fEdgeAlpha;
        float fBaseAlpha;
        ReadFloat(stream, fEdgeAlpha);
        ReadFloat(stream, fBaseAlpha);
        mBaseColor.a = fBaseAlpha;
        mEdgeColor.a = fEdgeAlpha;
    }
    mEnable = ReadFlag(stream);
    mVertBase = ReadFlag(stream);
    mVertLight = ReadFlag(stream);
    mVertEdge = ReadFlag(stream);
    int bOldVertBase = 0;
    int bOldVertEmissive = 0;
    if (sLoadRev < kRevBaseAmbient) {
        bOldVertBase = ReadFlag(stream);
        bOldVertEmissive = ReadFlag(stream);
    }
    int nCull;
    stream.ReadEndian(&nCull, sizeof(nCull));
    mCull = nCull;
    if (sLoadRev >= kRevMultiPassCount) {
        stream.ReadEndian(&mMultiPass, sizeof(mMultiPass));
    } else if (sLoadRev >= kRevMultiPassFlag) {
        mMultiPass = ReadFlag(stream);
    }
    if (sLoadRev >= kRevNormalize) {
        mNormalize = ReadFlag(stream);
    }
    if (sLoadRev >= kRevFlat) {
        mFlat = ReadFlag(stream);
    }
    if (sLoadRev >= kRevBaseAmbient) {
        mBaseAmbient = ReadFlag(stream);
    } else {
        if (oldBase.r != 0.0f || oldBase.g != 0.0f || oldBase.b != 0.0f || bOldVertBase != 0) {
            mBaseColor.r = oldBase.r;
            mBaseColor.b = oldBase.b;
            mBaseColor.g = oldBase.g;
            mVertBase = bOldVertBase;
            mBaseAmbient = 0;
        }
        mVertBase = (mVertBase != 0) | bOldVertEmissive;
    }
    Refresh();
}

void RndMat::SetLighting(
    int bEnable, int bVertBase, int bVertLight, int bVertEdge, int bNormalize, int bBaseAmbient) {
    mBaseAmbient = bBaseAmbient;
    mEnable = bEnable;
    mVertBase = bVertBase;
    mVertLight = bVertLight;
    mVertEdge = bVertEdge;
    mNormalize = bNormalize;
}

void RndMat::Refresh() {
    int nStage = 0;
    for (Stage &stage : mStages) {
        stage.mMat = this;
        if (stage.mTex != nullptr) {
            stage.mTex->AddRef(this);
        }
        SyncStage(nStage);
        ++nStage;
    }
}

void RndMat::AddStage() {
    const Stage stage;
    mStages.resize(mStages.size() + 1, stage);
    mStages.back().mMat = this;
}

void RndMat::ReleaseStageTex() {
    for (Stage &stage : mStages) {
        if (stage.mTex != nullptr) {
            stage.mTex->RemoveRef(this);
        }
    }
}

PrnStream &operator<<(PrnStream &stream, RndMat::Blend eBlend) {
    switch (eBlend) {
    case RndMat::kBlendDest:
        return stream << "Dest";
    case RndMat::kBlendSrc:
        return stream << "Src";
    case RndMat::kBlendAdd:
        return stream << "Add";
    case RndMat::kBlendMultiply:
        return stream << "Multiply";
    case RndMat::kBlendMultiply2:
        return stream << "Multiply2";
    case RndMat::kBlendSrcAlpha:
        return stream << "SrcAlpha";
    case RndMat::kBlendSrcAlphaAdd:
        return stream << "SrcAlphaAdd";
    case RndMat::kBlendSrcAdd:
        return stream << "SrcAdd";
    case RndMat::kBlendInvSrcAlpha:
        return stream << "InvSrcAlpha";
    case RndMat::kBlendDestAlpha:
        return stream << "DestAlpha";
    case RndMat::kBlendInvDestAlpha:
        return stream << "InvDestAlpha";
    case RndMat::kBlendSrcAlphaOpaque:
        return stream << "SrcAlphaOpaque";
    case RndMat::kBlendSrcAlphaCutout:
        return stream << "SrcAlphaCutout";
    case RndMat::kBlendSubtract:
        return stream << "Subtract";
    default:
        return stream;
    }
}

PrnStream &operator<<(PrnStream &stream, RndMat::TexGen eTexGen) {
    switch (eTexGen) {
    case RndMat::kTexGenFixed:
        return stream << "Fixed";
    case RndMat::kTexGenSphere:
        return stream << "Sphere";
    case RndMat::kTexGenPlanar:
        return stream << "Planar";
    case RndMat::kTexGenOrthoCube:
        return stream << "OrthoCube";
    case RndMat::kTexGenLocalCube:
        return stream << "LocalCube";
    default:
        return stream;
    }
}

PrnStream &operator<<(PrnStream &stream, RndMat::TexWrap eWrap) {
    switch (eWrap) {
    case RndMat::kTexWrapClamp:
        return stream << "Clamp";
    case RndMat::kTexWrapRepeat:
        return stream << "Repeat";
    case RndMat::kTexWrapMirror:
        return stream << "Mirror";
    default:
        return stream;
    }
}

PrnStream &operator<<(PrnStream &stream, const std::vector<RndMat::Stage> &stages) {
    stream << "(size:" << static_cast<unsigned int>(stages.size()) << ")";
    int nIndex = 0;
    for (const RndMat::Stage &stage : stages) {
        stream << "\n" << nIndex << "\t";
        stage.Print(stream);
        ++nIndex;
    }
    return stream;
}

BinStream &operator<<(BinStream &stream, const std::vector<RndMat::Stage> &stages) {
    const int nSize = static_cast<int>(stages.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (const RndMat::Stage &stage : stages) {
        stage.Save(stream);
    }
    return stream;
}

BinStream &operator>>(BinStream &stream, std::vector<RndMat::Stage> &stages) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    const RndMat::Stage stage;
    stages.resize(nSize, stage);
    for (RndMat::Stage &element : stages) {
        element.Load(stream);
    }
    return stream;
}

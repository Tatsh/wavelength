#include "rnd/rndmovie.h"

#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

// The first version with the texture.
constexpr int kRevTex = 4;

// The size of the buffer Load() reads the file path into.
constexpr int kMaxPath = 256;

} // namespace

const char *RndMovie::sClassName = "Movie";
int RndMovie::sRev = 4;

RndMovie::RndMovie(const char *pszName) : RndObject(pszName) {
    mTex = nullptr;
}

RndMovie::~RndMovie() {
    Close();
}

void RndMovie::ListAnimObjects(std::list<RndObject *> &objects) {
    objects.push_back(mTex);
    RndAnimatable::ListAnimObjects(objects);
}

void RndMovie::Open() {
    if (mTex != nullptr) {
        mTex->AddRef(this);
    }
}

void RndMovie::Close() {
    if (mTex != nullptr) {
        mTex->RemoveRef(this);
    }
}

void RndMovie::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndMovie]\n";
    stream << "file:" << mFile.RelativePath() << " tex:" << static_cast<const RndObject *>(mTex)
           << "\n";
}

void RndMovie::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    stream.WriteString(mFile.RelativePath());
    stream.WriteString(mTex != nullptr ? mTex->mName.c_str() : "");
}

void RndMovie::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
    if (mTex != pFrom || mTex == nullptr) {
        return;
    }
    mTex->RemoveRef(this);
    mTex = pTo != nullptr ? dynamic_cast<RndTex *>(pTo) : nullptr;
    if (mTex != nullptr) {
        mTex->AddRef(this);
    }
}

void RndMovie::Copy(const RndObject *pSource, int nFlags) {
    const RndMovie *pMovie = pSource != nullptr ? dynamic_cast<const RndMovie *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    Close();
    mFile = pMovie->mFile;
    mTex = pMovie->mTex;
    Open();
}

void RndMovie::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new Movie");
        return;
    }
    RndAnimatable::Load(stream);
    Close();
    char szFile[kMaxPath];
    stream.ReadString(szFile, sizeof(szFile));
    mFile.Set(szFile);
    if (nRev >= kRevTex) {
        String texName;
        stream >> texName;
        if (texName.mLength == 0) {
            mTex = nullptr;
        } else {
            RndObject *pObject = TheManager.Find(texName.c_str());
            mTex = pObject != nullptr ? dynamic_cast<RndTex *>(pObject) : nullptr;
        }
    }
    Open();
}

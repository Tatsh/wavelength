#include "game/torsopart.h"

#include "game/avatar.h"
#include "game/avatarpartset.h"
#include "os/string.h"

namespace {

// The loads of the emblem files.
constexpr int kLoaderFlags = RndLoader::kAsync | RndLoader::kDeleteObjects;

// A choice of the `types` array lists its file after its name.
constexpr int kEmblemFile = 1;

// The emblem takes the last stage of a material with at least two stages.
constexpr size_t kMinEmblemStages = 2;

} // namespace

TorsoPart::TorsoPart(int nParent, Avatar *pAvatar)
    : AvatarPart(AvatarPartSet::kPartTorso, nParent, pAvatar) {
    mPendingEmblem = Avatar::sNone;
    mEmblemTex = nullptr;
    mEmblemLoader = nullptr;
    mEmblem = Avatar::sNone;
}

TorsoPart::~TorsoPart() {
    delete mEmblemLoader;
    mEmblemLoader = nullptr;
}

void TorsoPart::SetEmblem(const char *pszEmblem) {
    if (mEmblem == pszEmblem) {
        return;
    }
    mPendingEmblem = pszEmblem;
    DataArray *pEmblems = mData->FindArray("emblem", false);
    const char *pszDir;
    pEmblems->FindSymbol("rnd_dir", &pszDir, true);
    DataArray *pType = pEmblems->FindArray("types", true)->FindArray(pszEmblem, true);
    const char *pszFile = FormatString("%s/%s.rnd", pszDir, pType->Sym(kEmblemFile));
    delete mEmblemLoader;
    mEmblemLoader = nullptr;
    mEmblemLoader = TheManager.AddLoader(pszFile, kLoaderFlags, &mCallback, nullptr);
}

bool TorsoPart::Rebuild() {
    if (mType == mPendingType) {
        return true;
    }
    if (!AvatarPart::Rebuild()) {
        return false;
    }
    ApplyEmblem();
    return true;
}

void TorsoPart::ReleaseType() {
    AvatarPart::ReleaseType();
    delete mEmblemLoader;
    mEmblemLoader = nullptr;
    mPendingEmblem = mEmblem;
}

bool TorsoPart::RebuildEmblem() {
    if (mEmblem == mPendingEmblem) {
        return true;
    }
    if (!mEmblemLoader->IsLoaded()) {
        return false;
    }
    delete mEmblemTex;
    mEmblemTex = nullptr;
    std::list<RndObject *> &objects = mEmblemLoader->mObjects;
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        mEmblemTex = dynamic_cast<RndTex *>(*it);
        if (mEmblemTex != nullptr) {
            // Removing the texture from the list prevents the loader from deleting it.
            objects.erase(it);
            break;
        }
    }
    delete mEmblemLoader;
    mEmblemLoader = nullptr;
    // The binary does not check for a file without a texture.
    mEmblemTex->SetName(FormatString("avatar%d_%s", mAvatar->mIndex, mEmblemTex->mName.c_str()));
    mEmblem = mPendingEmblem;
    ApplyEmblem();
    return true;
}

void TorsoPart::ApplyEmblem() {
    std::vector<RndMat::Stage> &stages = mMat->mStages;
    if (stages.size() < kMinEmblemStages) {
        return;
    }
    RndMat::Stage &stage = stages[stages.size() - 1];
    if (mEmblem != Avatar::sNone) {
        stage.SetTex(mEmblemTex);
        stage.mBlend = RndMat::kBlendSrcAlpha;
    } else {
        stage.mBlend = RndMat::kBlendDest;
    }
}

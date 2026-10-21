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
    mPendingEmblem = Avatar::GetNone();
    mEmblem = Avatar::GetNone();
}

TorsoPart::~TorsoPart() = default;

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
    mEmblemLoader.reset();
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
    mEmblemLoader.reset();
    mPendingEmblem = mEmblem;
}

bool TorsoPart::RebuildEmblem() {
    if (mEmblem == mPendingEmblem) {
        return true;
    }
    if (!mEmblemLoader->IsLoaded()) {
        return false;
    }
    // Preserve the original deletion point, before the new texture is taken from the loader.
    mEmblemTex.reset();
    std::list<RndObject *> &objects = mEmblemLoader->GetObjects();
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        // Only the texture is taken over; the other objects stay with the loader.
        mEmblemTex.reset(dynamic_cast<RndTex *>(*it));
        if (mEmblemTex != nullptr) {
            // Removing the texture from the list prevents the loader from deleting it.
            objects.erase(it);
            break;
        }
    }
    mEmblemLoader.reset();
    // The binary does not check for a file without a texture.
    mEmblemTex->SetName(FormatString("avatar%d_%s", mAvatar->GetIndex(), mEmblemTex->GetName()));
    mEmblem = mPendingEmblem;
    ApplyEmblem();
    return true;
}

void TorsoPart::ApplyEmblem() {
    std::vector<RndMat::Stage> &stages = mMat->GetStages();
    if (stages.size() < kMinEmblemStages) {
        return;
    }
    RndMat::Stage &stage = stages[stages.size() - 1];
    if (mEmblem != Avatar::GetNone()) {
        stage.SetTex(mEmblemTex.get());
        stage.SetBlend(RndMat::kBlendSrcAlpha);
    } else {
        stage.SetBlend(RndMat::kBlendDest);
    }
}

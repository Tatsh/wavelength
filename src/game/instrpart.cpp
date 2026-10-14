#include "game/instrpart.h"

#include "game/avatar.h"
#include "game/avatarpartset.h"
#include "os/string.h"

namespace {

// The load of the instrument file.
constexpr int kLoaderFlags =
    RndLoader::kAsync | RndLoader::kReportExisting | RndLoader::kDeleteObjects;

// A choice of the `types` array gives the number of its mesh after its name.
constexpr int kMeshNumber = 1;

} // namespace

InstrPart::InstrPart(int nParent, Avatar *pAvatar)
    : AvatarPart(AvatarPartSet::kPartInstrument, nParent, pAvatar) {
}

void InstrPart::SetType(const char *pszType) {
    if (mType == pszType) {
        return;
    }
    mPendingType = pszType;
    if (mLoader == nullptr) {
        const char *pszFile;
        mData->FindSymbol("rnd_dir", &pszFile, true);
        mLoader = TheManager.AddLoader(pszFile, kLoaderFlags, nullptr, nullptr);
        mRenamed = 0;
    }
    Rebuild(); // The binary discards the result.
}

bool InstrPart::Rebuild() {
    if (mType == mPendingType) {
        return true;
    }
    if (!mLoader->IsLoaded()) {
        return false;
    }
    if (mRenamed == 0) {
        mRenamed = 1;
        for (RndObject *pObject : mLoader->mObjects) {
            pObject->SetName(FormatString("avatar%d_%s", mAvatar->mIndex, pObject->mName.c_str()));
        }
    }
    mType = mPendingType;
    DataArray *pType = mData->FindArray("types", true)->FindArray(mType, true);
    mMesh = FindMesh(
        FormatString("avatar%d_instr_%02d.mesh", mAvatar->mIndex, pType->Int(kMeshNumber)));
    mTransRoot = mMesh;
    ResetLocalXfm(mTransRoot);
    if (!mAnims.empty()) {
        mAnims[0]->SetTrans(mTransRoot);
    }
    mMat = mMesh->mMat;
    if (mMat != nullptr) {
        mMat->SetLightColor(mColor);
    }
    return true;
}

void InstrPart::ReleaseType() {
    mPendingType = mType;
}

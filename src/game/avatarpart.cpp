#include "game/avatarpart.h"

#include <cstring>
#include <utility>

#include "game/avatar.h"
#include "game/avatarpartset.h"
#include "math/transform.h"
#include "os/debug.h"
#include "os/string.h"
#include "os/system.h"

namespace {

// The first node of the `anims` array is its tag.
constexpr int kFirstAnim = 1;

// A choice of the `types` array lists its first file, then a second file or an empty symbol.
constexpr int kFirstFile = 1;
constexpr int kSecondFile = 2;

// The loads of the files of a part.
constexpr int kLoaderFlags = RndLoader::kAsync | RndLoader::kDeleteObjects;

// The bones that retain the translation keys of the shared animation, the pelvis and the arm
// bones, whose names start with a side letter and `arm_`.
const char *const kPelvisBone = "pelvis";
const char *const kArmBonePrefix = "arm_";
constexpr int kArmBonePrefixLength = 4;
constexpr int kSideLetterLength = 1;

// The results of the load callback.
constexpr int kCreateObject = 1;
constexpr int kStopLoad = -1;

} // namespace

AvatarPart::AvatarPart(int nPart, int nParent, Avatar *pAvatar)
    : mCallback(this), mType(nullptr), mPendingType(nullptr), mColor{1.0f, 1.0f, 1.0f, 1.0f},
      mMesh(nullptr), mTransRoot(nullptr), mJoint(nullptr), mMat(nullptr), mParent(nullptr),
      mAvatar(pAvatar) {
    mData = SystemConfig()
                ->FindArray("db", true)
                ->FindArray("avatar", true)
                ->FindArray("parts", true)
                ->FindArray(nPart);
    if (nParent != AvatarPartSet::kNumParts) {
        mParent = pAvatar->GetPart(nParent);
    }
    DataArray *pAnims = mData->FindArray("anims", false);
    if (pAnims == nullptr) {
        mAnims.resize(0);
        return;
    }
    mAnims.resize(pAnims->Size() - kFirstAnim);
    for (size_t i = 0; i < mAnims.size(); ++i) {
        const char *pszName = pAnims->Sym(static_cast<int>(i) + kFirstAnim);
        RndTransAnim *pBase =
            dynamic_cast<RndTransAnim *>(TheManager.Find(FormatString("fbase_%s.tnm", pszName)));
        // The part created the animation through TheManager and deletes it.
        mAnims[i].reset(dynamic_cast<RndTransAnim *>(
            TheManager.Create(RndTransAnim::sClassName,
                              FormatString("avatar%d_%s.tnm", mAvatar->GetIndex(), pszName))));
        mAnims[i]->SetTransInterp(RndTransAnim::kInterpLinear);
        mAnims[i]->SetFramesOwner(pBase);
        // The keys belong to the shared `fbase_` animation, and every avatar clears them again.
        mAnims[i]->GetFramesOwner()->GetScaleKeys().clear();
        if (std::strcmp(pszName, kPelvisBone) != 0 &&
            std::strncmp(pszName + kSideLetterLength, kArmBonePrefix, kArmBonePrefixLength) != 0) {
            mAnims[i]->GetFramesOwner()->GetTransKeys().clear();
        }
    }
}

AvatarPart::~AvatarPart() {
    // Preserve the original forward deletion order of the animations.
    for (size_t i = 0; i < mAnims.size(); ++i) {
        mAnims[i].reset();
    }
}

void AvatarPart::SetType(const char *pszType) {
    if (pszType == mType) {
        return;
    }
    mPendingType = pszType;
    const char *pszDir;
    mData->FindSymbol("rnd_dir", &pszDir, true);
    DataArray *pType = mData->FindArray("types", true)->FindArray(pszType, true);
    mPendingLoader.reset();
    mPendingExtraLoader.reset();
    mPendingLoader = TheManager.AddLoader(FormatString("%s/%s.rnd", pszDir, pType->Sym(kFirstFile)),
                                          kLoaderFlags,
                                          &mCallback,
                                          nullptr);
    if (pType->Sym(kSecondFile)[0] != '\0') {
        mPendingExtraLoader =
            TheManager.AddLoader(FormatString("%s/%s.rnd", pszDir, pType->Sym(kSecondFile)),
                                 kLoaderFlags,
                                 &mCallback,
                                 nullptr);
    }
}

void AvatarPart::ReleaseType() {
    mPendingLoader.reset();
    mPendingExtraLoader.reset();
    mPendingType = mType;
}

void AvatarPart::SetMatTree(RndMesh *pMesh) {
    pMesh->SetMat(mMat);
    for (RndDrawable *pChild : pMesh->GetDraws()) {
        // A child that is not a mesh is passed on as null, as in the binary.
        SetMatTree(dynamic_cast<RndMesh *>(pChild));
    }
}

void AvatarPart::Draw() {
    if (mMesh != nullptr) {
        mMesh->Draw();
    }
}

void AvatarPart::SetBlend(float fBlend) {
    if (mMat != nullptr) {
        mMat->SetAlpha(fBlend);
    }
}

void AvatarPart::SetFrame(float fFrame) {
    const int nAnims = static_cast<int>(mAnims.size());
    for (int i = 0; i < nAnims; ++i) {
        mAnims[i]->SetFrame(fFrame);
    }
}

void AvatarPart::SetFrames(float fFrom, float fTo, float fBlend) {
    for (size_t i = 0; i < mAnims.size(); ++i) {
        RndTransAnim *pAnim = mAnims[i].get();
        if (!pAnim->GetFramesOwner()->GetTransKeys().empty()) {
            Vector3 translation;
            BlendTranslation(pAnim, fFrom, fTo, fBlend, translation);
            pAnim->GetTrans()->SetDirty(1);
            pAnim->GetTrans()->GetLocalXfm().mTranslation = translation;
        }
        Quat rotation;
        BlendRotation(pAnim, fFrom, fTo, fBlend, rotation);
        Transform basis;
        Rnd::MakeRotMatrix(rotation, &basis.mBasisX.x);
        pAnim->GetTrans()->GetLocalXfm().mBasisX = basis.mBasisX;
        pAnim->GetTrans()->GetLocalXfm().mBasisY = basis.mBasisY;
        pAnim->GetTrans()->GetLocalXfm().mBasisZ = basis.mBasisZ;
        pAnim->GetTrans()->SetDirty(1);
    }
}

bool AvatarPart::Rebuild() {
    if (mType == mPendingType) {
        return true;
    }
    if (mPendingLoader != nullptr) {
        if (!mPendingLoader->IsLoaded()) {
            return false;
        }
        if (mPendingExtraLoader != nullptr && !mPendingExtraLoader->IsLoaded()) {
            return false;
        }
        mLoader = std::move(mPendingLoader);
        if (mPendingExtraLoader != nullptr) {
            mLoader->GetObjects().splice(mLoader->GetObjects().end(),
                                         mPendingExtraLoader->GetObjects());
            mPendingExtraLoader.reset();
        }
        for (RndObject *pObject : mLoader->GetObjects()) {
            pObject->SetName(FormatString("avatar%d_%s", mAvatar->GetIndex(), pObject->GetName()));
        }
    }

    const char *pszRoot;
    mData->FindSymbol("draw_root", &pszRoot, true);
    mMesh = FindMesh(FormatString("avatar%d_%s.mesh", mAvatar->GetIndex(), pszRoot));
    mData->FindSymbol("trans_root", &pszRoot, true);
    mTransRoot = FindMesh(FormatString("avatar%d_%s.mesh", mAvatar->GetIndex(), pszRoot));
    ResetLocalXfm(mTransRoot);

    if (mLoader != nullptr) {
        mMat = nullptr;
        for (RndObject *pLoaded : mLoader->GetObjects()) {
            mMat = dynamic_cast<RndMat *>(pLoaded);
            if (mMat != nullptr) {
                break;
            }
        }
        mMat->SetLightColor(mColor); // The binary does not check for a file without a material.
    }
    SetMatTree(mMesh);

    DataArray *pAnims = mData->FindArray("anims", false);
    if (pAnims != nullptr) {
        for (size_t i = 0; i < mAnims.size(); ++i) {
            mAnims[i]->SetTrans(
                FindMesh(FormatString("avatar%d_%s.mesh",
                                      mAvatar->GetIndex(),
                                      pAnims->Sym(static_cast<int>(i) + kFirstAnim))));
        }
    }
    for (int i = 0; i < AvatarPartSet::kNumParts; ++i) {
        AvatarPart *pPart = mAvatar->GetPart(i);
        if (pPart->mParent == this) {
            pPart->FindJoint();
        }
    }
    mType = mPendingType;
    return true;
}

void AvatarPart::Update() {
    if (mJoint != nullptr && mTransRoot != nullptr) {
        mTransRoot->UpdateWorldXfm(mJoint, 1);
    }
}

void AvatarPart::FindJoint() {
    const char *pszRoot;
    mData->FindSymbol("trans_root", &pszRoot, true);
    mJoint = FindMesh(FormatString("avatar%d_%s_joint.mesh", mAvatar->GetIndex(), pszRoot));
}

void AvatarPart::SetColor(const Color *pColor) {
    mColor.r = pColor->r;
    mColor.g = pColor->g;
    mColor.b = pColor->b;
    if (mMat != nullptr) {
        mMat->SetLightColor(mColor);
    }
}

int AvatarPart::Callback::ShouldLoad(RndObject *pExisting,
                                     [[maybe_unused]] const char *pszName,
                                     [[maybe_unused]] const char *pszClass) {
    if (pExisting == nullptr) {
        return kCreateObject;
    }
    DebugPrint("Resetting avatar because found %s\n", pExisting->GetName());
    mOwner->SetType(mOwner->GetPendingType());
    return kStopLoad;
}

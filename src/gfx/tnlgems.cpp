#include "gfx/tnlgems.h"

#include <cmath>
#include <iterator>

#include "game/gamedb.h"
#include "game/triggermgr.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "gfx/meshrotation.h"
#include "os/debug.h"
#include "os/string.h"
#include "rnd/drawable.h"
#include "rnd/manager.h"
#include "rnd/meshanim.h"
#include "rnd/multimesh.h"
#include "rnd/particlesys.h"

namespace {

// The transform pool and the particle pools take this many beyond the most gems placed at once.
constexpr int kSpareCount = 64;

// A gem behind the song by this many ticks is dropped, and a gem this far ahead is not kept.
constexpr float kDefaultTrailTicks = 960.0f;
constexpr float kHorizonTicks = 23040.0f;

// A gem fires its trigger events this many ticks before the song reaches it.
constexpr float kPassTicks = -20.0f;

constexpr char kNoTrack = -1;
constexpr int kFocusPlayer = 0;
constexpr char kFirstCompositeLetter = 'a';
constexpr int kLoopRotation = 1;

// NTSC-U/C: 0x003afbb8
float sMeshScale = 1.0f;
// NTSC-U/C: 0x003afbbc
float sSpriteScale = 1.0f;
// NTSC-U/C: 0x0043b650
std::map<Rnd::Object *, float> sRenderScales;

} // namespace

TnlGems::TnlGems(GfxTunnel *pTunnel, int nMaxGems, int)
    : mTunnel(pTunnel), mTrailTicks(kDefaultTrailTicks), mMaxGems(nMaxGems), mFocusTrack(kNoTrack),
      mHasFocusTrack(0) {
    // The original fills the pool from an uninitialised temporary.
    for (int i = nMaxGems + kSpareCount; i != 0; --i) {
        mTransforms.insert(mTransforms.begin(), Transform());
    }
}

TnlGems::~TnlGems() {
    for (auto *pGroup : mMeshGroups) {
        delete pGroup;
    }
    for (auto *pGroup : mSpriteGroups) {
        delete pGroup;
    }
    while (!mObjects.empty()) {
        delete mObjects.front();
        mObjects.pop_front();
    }
}

void TnlGems::LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *, int) {
    FindConfigFloat(pConfig, pDefaults, "gem_particle_height", &TnlGem::sParticleHeight, true);
    FindConfigFloat(pConfig, pDefaults, "sprite_gem_height", &TnlGem::sSpriteHeight, true);
    const float flScale = TheGfxManager.mScrollSpeed;
    const float flSpriteDepth = flScale * TnlGem::sSpriteHeight;
    TnlGem::sParticleHeight *= flScale;
    TnlGem::sSpriteHeight = 1.0f - flSpriteDepth;
    FindConfigFloat(pConfig, pDefaults, "gem_mesh_flash_size", &TnlGem::sMeshFlashSize, true);
    TnlGem::sMeshFlashSize *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "gem_mesh_flash_time", &TnlGem::sMeshFlashRate, true);
    TnlGem::sMeshFlashRate = TheGfxManager.mScrollSpeed / TnlGem::sMeshFlashRate;
    FindConfigFloat(pConfig, pDefaults, "gem_flare_flash_size", &TnlGem::sFlareFlashSize, true);
    FindConfigFloat(pConfig, pDefaults, "gem_flare_flash_time", &TnlGem::sFlareFlashRate, true);
    TnlGem::sFlareFlashRate = 1.0f / TnlGem::sFlareFlashRate;
    FindConfigFloat(pConfig, pDefaults, "mesh_gem_scale", &sMeshScale, true);
    FindConfigFloat(pConfig, pDefaults, "sprite_gem_scale", &sSpriteScale, true);
}

void TnlGems::SetRenderScale(Rnd::Object *pObject, float flScale) {
    sRenderScales[pObject] = flScale;
}

void TnlGems::SetRenderScale(Rnd::Mesh *pMesh, float flScale) {
    sRenderScales[pMesh->mVertsOwner] = flScale;
}

void TnlGems::ApplyRenderScales(float flWorldScale) {
    Transform scale;
    scale.mBasisX = Vector3{1.0f, 0.0f, 0.0f};
    scale.mBasisY = Vector3{0.0f, 1.0f, 0.0f};
    scale.mBasisZ = Vector3{0.0f, 0.0f, 1.0f};
    scale.mTranslation = Vector3{0.0f, 0.0f, 0.0f};
    for (auto it = sRenderScales.begin(); it != sRenderScales.end(); ++it) {
        Rnd::Object *pObject = it->first;
        if (auto *pMesh = dynamic_cast<Rnd::Mesh *>(pObject)) {
            const float flScale = it->second * flWorldScale;
            scale.mBasisZ.z = flWorldScale;
            scale.mBasisX.x = flScale;
            scale.mBasisY.y = flScale;
            pMesh->TransformVerts(scale);
        } else if (auto *pSys = dynamic_cast<Rnd::ParticleSys *>(pObject)) {
            pSys->mSizeLow = (pSys->mSizeLow * it->second) * flWorldScale;
            pSys->mSizeHigh = (pSys->mSizeHigh * it->second) * flWorldScale;
        } else {
            DebugWarn("unkown object type: %s", pObject->ClassName().mStr);
        }
    }
    sRenderScales.clear();
}

void TnlGems::AddRotation(Rnd::Mesh *pMesh,
                          std::list<Rnd::Object *> *pObjects,
                          int nKeys,
                          int bLoop,
                          Rnd::Animatable *pParent,
                          float flStartFrame,
                          float flStartAngle,
                          float flEndFrame,
                          float flEndAngle) {
    const char *pszName = FormatString("%s rot.msnm", pMesh->mName.mStr);
    if (dynamic_cast<Rnd::MeshAnim *>(Rnd::TheManager.Find(pszName)) != nullptr) {
        return;
    }
    auto *pAnim = dynamic_cast<Rnd::MeshAnim *>(
        Rnd::TheManager.Create(Rnd::g_meshAnimClassName.mStr, pszName));
    pObjects->push_back(pAnim);
    pAnim->SetMesh(pMesh);
    AddRotationKeys(pMesh, pAnim, nKeys, flStartFrame, flStartAngle, flEndFrame, flEndAngle);
    if (bLoop) {
        pAnim->AddMinMaxLoop(flStartFrame, flEndFrame, kLoopRotation);
    }
    pParent->AddAnim(pAnim);
}

char TnlGems::AddMeshGroups(const char *pszName, const char *pszDrawParent, float flLookahead) {
    const int nFirstType = static_cast<int>(mMeshGroups.size());
    Rnd::Drawable *pParent = nullptr;
    if (pszDrawParent != nullptr) {
        pParent = dynamic_cast<Rnd::Drawable *>(Rnd::TheManager.Find(pszDrawParent));
    }
    MeshGroup *pPrev = nullptr;
    for (int n = 0;; ++n) {
        auto *pMultiMesh = dynamic_cast<Rnd::MultiMesh *>(
            Rnd::TheManager.Find(FormatString("%s%d.mm", pszName, n)));
        if (pMultiMesh == nullptr) {
            break;
        }
        if (pParent != nullptr) {
            pParent->AddDraw(pMultiMesh, nullptr);
        }
        SetRenderScale(pMultiMesh->GetMesh(), sMeshScale);
        auto *pGroup =
            new MeshGroup(pMultiMesh, nullptr, nullptr, &mTransforms, mMaxGems, flLookahead);
        mMeshGroups.push_back(pGroup);
        if (pPrev != nullptr) {
            pPrev->mNext = pGroup;
        }
        pPrev = pGroup;
    }
    return static_cast<char>(nFirstType);
}

char TnlGems::AddCompositeMeshGroups(const char *pszName,
                                     const char *pszDrawParent,
                                     float flLookahead) {
    const int nFirstType = static_cast<int>(mMeshGroups.size());
    Rnd::Drawable *pParent = nullptr;
    if (pszDrawParent != nullptr) {
        pParent = dynamic_cast<Rnd::Drawable *>(Rnd::TheManager.Find(pszDrawParent));
    }
    std::list<Rnd::Mesh *> meshes;
    MeshGroup *pPrev = nullptr;
    for (int n = 0;; ++n) {
        auto *pMultiMesh = dynamic_cast<Rnd::MultiMesh *>(
            Rnd::TheManager.Find(FormatString("%s_%d.mm", pszName, n)));
        if (pMultiMesh == nullptr) {
            break;
        }
        if (pParent != nullptr) {
            pParent->AddDraw(pMultiMesh, nullptr);
        }
        for (char nLetter = kFirstCompositeLetter;; ++nLetter) {
            auto *pMesh = dynamic_cast<Rnd::Mesh *>(
                Rnd::TheManager.Find(FormatString("%s%c_%d.mesh", pszName, nLetter, n)));
            if (pMesh == nullptr) {
                break;
            }
            SetRenderScale(pMesh, sMeshScale);
            meshes.push_back(pMesh);
        }
        auto *pSys =
            dynamic_cast<Rnd::ParticleSys *>(Rnd::TheManager.Find(FormatString("%s.ps", pszName)));
        if (pSys != nullptr) {
            SetRenderScale(static_cast<Rnd::Object *>(pSys), sMeshScale);
        }
        auto *pGroup =
            new MeshGroup(pMultiMesh, &meshes, pSys, &mTransforms, mMaxGems, flLookahead);
        mMeshGroups.push_back(pGroup);
        if (pPrev != nullptr) {
            pPrev->mNext = pGroup;
        }
        pPrev = pGroup;
        meshes.clear();
    }
    return static_cast<char>(nFirstType);
}

char TnlGems::AddSpriteGroup(const char *pszName) {
    const int nType = static_cast<int>(mSpriteGroups.size()) + TnlGem::kTypeSprite;
    mSpriteGroups.push_back(new SpriteGroup(pszName, mMaxGems));
    Rnd::ParticleSys *pSys = mSpriteGroups.back()->mParticles;
    if (pSys != nullptr) {
        SetRenderScale(static_cast<Rnd::Object *>(pSys), sSpriteScale);
    }
    return static_cast<char>(nType);
}

void TnlGems::AddRotations(char nType,
                           int nKeys,
                           int bLoop,
                           Rnd::Animatable *pParent,
                           float flStartFrame,
                           float flStartAngle,
                           float flEndFrame,
                           float flEndAngle) {
    for (MeshGroup *pGroup = GetMeshGroup(nType); pGroup != nullptr; pGroup = pGroup->mNext) {
        if (pGroup->mMeshes.empty()) {
            AddRotation(pGroup->mMultiMesh->GetMesh(),
                        &mObjects,
                        nKeys,
                        bLoop,
                        pParent,
                        flStartFrame,
                        flStartAngle,
                        flEndFrame,
                        flEndAngle);
        } else {
            for (auto *pMesh : pGroup->mMeshes) {
                AddRotation(pMesh,
                            &mObjects,
                            nKeys,
                            bLoop,
                            pParent,
                            flStartFrame,
                            flStartAngle,
                            flEndFrame,
                            flEndAngle);
            }
        }
    }
}

void TnlGems::ClearInstances(const char *pszName) {
    for (int n = 0;; ++n) {
        auto *pMultiMesh = dynamic_cast<Rnd::MultiMesh *>(
            Rnd::TheManager.Find(FormatString("%s%d.mm", pszName, n)));
        if (pMultiMesh == nullptr) {
            break;
        }
        pMultiMesh->GetTransforms().clear();
    }
}

void TnlGems::ApplyRenderScales() {
    ApplyRenderScales(TheGfxManager.mScrollSpeed);
}

MeshGroup *TnlGems::GetMeshGroup(char nType) {
    return mMeshGroups[nType];
}

SpriteGroup *TnlGems::GetSpriteGroup(char nType) {
    return mSpriteGroups[static_cast<char>(nType - TnlGem::kTypeSprite)];
}

void TnlGems::AddGem(
    char nTrack, char nType, char nSlot, int bFlash, char nColor, float flTick, float flShowTick) {
    auto it = FindNearest(flTick);
    for (; it != mGems.end() && it->mTick <= flTick; ++it) {
        if (flTick == it->mTick && nSlot == it->mSlot && nTrack == it->mTrack) {
            it->mRemoveTick = flShowTick;
        }
    }
    TnlGem &gem = *mGems.insert(it, TnlGem());
    gem.mType = nType;
    gem.mTrack = nTrack;
    gem.mTick = flTick;
    gem.mSlot = nSlot;
    gem.mFlags = bFlash ? TnlGem::kFlagFlash : TnlGem::kFlagNoFlash;
    gem.mShowTick = flShowTick;
    if (nColor >= 0) {
        gem.mFlags |= nColor | TnlGem::kFlagPlayerColor;
    }
}

void TnlGems::RemoveGems(char nTrack, float flStartTick, float flEndTick) {
    auto it = FindNearest(flStartTick);
    while (it != mGems.end() && it->mTick < flEndTick) {
        if (it->mTrack == nTrack) {
            it->Release(mTunnel);
            it = mGems.erase(it);
        } else {
            ++it;
        }
    }
}

void TnlGems::RemoveGem(char nTrack, char nSlot, float flTick) {
    auto it = FindNearest(flTick);
    while (it != mGems.end() && it->mTick == flTick) {
        if (it->mSlot == nSlot && it->mTrack == nTrack) {
            it->Release(mTunnel);
            it = mGems.erase(it);
        } else {
            ++it;
        }
    }
}

void TnlGems::Clear() {
    auto it = mGems.begin();
    while (it != mGems.end()) {
        it->Release(nullptr);
        it = mGems.erase(it);
    }
}

void TnlGems::Poll(const TnlTrackRange *pRange) {
    const float flTick = TheGameDb->mSongTick;
    const char nFocusTrack = mHasFocusTrack ? mFocusTrack : kNoTrack;
    const float flTrailTick = flTick - mTrailTicks;
    auto it = mGems.begin();
    while (it != mGems.end() && it->mTick < flTrailTick) {
        it->Release(nullptr);
        it = mGems.erase(it);
    }

    const float flHorizonTick = flTick + kHorizonTicks;
    int nPlaced = 0;
    while (it != mGems.end()) {
        if (it->mRemoveTick < flTick || flHorizonTick < it->mTick) {
            it->Release(mTunnel);
            it = mGems.erase(it);
            continue;
        }
        if ((it->mFlags & TnlGem::kFlagPassed) == 0 && kPassTicks < flTick - it->mTick) {
            if ((it->mFlags & TnlGem::kFlagFlash) != 0) {
                it->Flash(mTunnel);
            }
            it->mFlags |= TnlGem::kFlagPassed;
            TheTriggerMgr.GemEvent(it->mTrack, it->mSlot);
            if (it->mTrack == nFocusTrack) {
                TheTriggerMgr.HitEvent(kFocusPlayer, it->mSlot);
            }
        }
        if (nPlaced < mMaxGems) {
            if (it->mShowTick < flTick) {
                it->Update(this, mTunnel, pRange, flTick);
                ++nPlaced;
            }
        } else {
            it->Release(nullptr);
        }
        ++it;
    }
}

void TnlGems::DrawComposites() {
    for (auto *pGroup : mMeshGroups) {
        if (pGroup->mMeshes.empty()) {
            continue;
        }
        Rnd::MultiMesh *pMultiMesh = pGroup->mMultiMesh;
        auto it = pGroup->mMeshes.begin();
        pMultiMesh->SetMesh(*it);
        pMultiMesh->Draw();
        ++it;
        if (pGroup->mParticles != nullptr) {
            pGroup->mParticles->Draw();
        }
        for (; it != pGroup->mMeshes.end(); ++it) {
            pMultiMesh->SetMesh(*it);
            pMultiMesh->Draw();
        }
    }
}

std::list<TnlGem>::iterator TnlGems::LowerBound(float flTick) {
    return FindLowerBound(flTick);
}

std::list<TnlGem>::iterator TnlGems::Find(float flTick) {
    return FindNearest(flTick);
}

std::list<TnlGem>::iterator TnlGems::FindNearest(float flTick) {
    // The original repeats the body of FindLowerBound() here.
    return FindLowerBound(flTick);
}

std::list<TnlGem>::iterator TnlGems::FindLowerBound(float flTick) {
    auto first = mGems.begin();
    if (first == mGems.end()) {
        return first;
    }
    auto it = std::prev(mGems.end());
    if (it->mTick < flTick) {
        return mGems.end();
    }
    if (std::fabs(it->mTick - flTick) < std::fabs(first->mTick - flTick)) {
        while (it != first) {
            const auto prev = std::prev(it);
            if (prev->mTick < flTick) {
                return it;
            }
            it = prev;
        }
        return first;
    }
    for (; first != mGems.end(); ++first) {
        if (flTick <= first->mTick) {
            break;
        }
    }
    return first;
}

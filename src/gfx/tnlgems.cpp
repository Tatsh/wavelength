#include "gfx/tnlgems.h"

#include <cmath>
#include <iterator>
#include <memory>

#include "game/gamedb.h"
#include "game/triggermgr.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "gfx/meshrotation.h"
#include "gfx/unreadvector.h"
#include "os/debug.h"
#include "os/string.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmeshanim.h"
#include "rnd/rndmultimesh.h"
#include "rnd/rndparticlesys.h"

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

// NTSC-U/C: 0x003afbb8, PAL: 0x0042ea50
float sMeshScale = 1.0f;
// NTSC-U/C: 0x003afbbc, PAL: 0x0042ea54
float sSpriteScale = 1.0f;

// The static initialiser of the unit, and the routines that run it to construct and to destroy the
// render scales of the gems.
// NTSC-U/C: 0x001f6100, PAL: 0x001feea0
// NTSC-U/C: 0x001f6208, PAL: 0x001fefa8
// NTSC-U/C: 0x001f6228, PAL: 0x001fefc8

// NTSC-U/C: 0x0043b650
std::map<RndObject *, float> sRenderScales;

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
    // The groups go before the animations, as in the binary.
    mMeshGroups.clear();
    mSpriteGroups.clear();
}

void TnlGems::LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *, int) {
    FindConfigFloat(pConfig, pDefaults, "gem_particle_height", &TnlGem::sParticleHeight, true);
    FindConfigFloat(pConfig, pDefaults, "sprite_gem_height", &TnlGem::sSpriteHeight, true);
    const float flScale = TheGfxManager.GetScrollSpeed();
    const float flSpriteDepth = flScale * TnlGem::sSpriteHeight;
    TnlGem::sParticleHeight *= flScale;
    TnlGem::sSpriteHeight = 1.0f - flSpriteDepth;
    FindConfigFloat(pConfig, pDefaults, "gem_mesh_flash_size", &TnlGem::sMeshFlashSize, true);
    TnlGem::sMeshFlashSize *= TheGfxManager.GetScrollSpeed();
    FindConfigFloat(pConfig, pDefaults, "gem_mesh_flash_time", &TnlGem::sMeshFlashRate, true);
    TnlGem::sMeshFlashRate = TheGfxManager.GetScrollSpeed() / TnlGem::sMeshFlashRate;
    FindConfigFloat(pConfig, pDefaults, "gem_flare_flash_size", &TnlGem::sFlareFlashSize, true);
    FindConfigFloat(pConfig, pDefaults, "gem_flare_flash_time", &TnlGem::sFlareFlashRate, true);
    TnlGem::sFlareFlashRate = 1.0f / TnlGem::sFlareFlashRate;
    FindConfigFloat(pConfig, pDefaults, "mesh_gem_scale", &sMeshScale, true);
    FindConfigFloat(pConfig, pDefaults, "sprite_gem_scale", &sSpriteScale, true);
}

void TnlGems::SetRenderScale(RndObject *pObject, float flScale) {
    sRenderScales[pObject] = flScale;
}

void TnlGems::SetRenderScale(RndMesh *pMesh, float flScale) {
    sRenderScales[pMesh->GetGeomOwner()] = flScale;
}

void TnlGems::ApplyRenderScales(float flWorldScale) {
    Transform scale;
    scale.mBasisX = Vector3{1.0f, 0.0f, 0.0f};
    scale.mBasisY = Vector3{0.0f, 1.0f, 0.0f};
    scale.mBasisZ = Vector3{0.0f, 0.0f, 1.0f};
    scale.mTranslation = Vector3{0.0f, 0.0f, 0.0f};
    for (auto it = sRenderScales.begin(); it != sRenderScales.end(); ++it) {
        RndObject *pObject = it->first;
        if (auto *pMesh = dynamic_cast<RndMesh *>(pObject)) {
            const float flScale = it->second * flWorldScale;
            scale.mBasisZ.z = flWorldScale;
            scale.mBasisX.x = flScale;
            scale.mBasisY.y = flScale;
            pMesh->TransformVerts(scale);
        } else if (auto *pSys = dynamic_cast<RndParticleSys *>(pObject)) {
            pSys->GetSize().x = (pSys->GetSize().x * it->second) * flWorldScale;
            pSys->GetSize().y = (pSys->GetSize().y * it->second) * flWorldScale;
        } else {
            DebugWarn("unkown object type: %s", pObject->ClassName());
        }
    }
    sRenderScales.clear();
}

void TnlGems::AddRotation(RndMesh *pMesh,
                          std::list<std::unique_ptr<RndObject>> *pObjects,
                          int nKeys,
                          int bLoop,
                          RndAnimatable *pParent,
                          float flStartFrame,
                          float flStartAngle,
                          float flEndFrame,
                          float flEndAngle) {
    const char *pszName = FormatString("%s rot.msnm", pMesh->GetName());
    if (dynamic_cast<RndMeshAnim *>(TheManager.Find(pszName)) != nullptr) {
        return;
    }
    auto *pAnim = dynamic_cast<RndMeshAnim *>(TheManager.Create(RndMeshAnim::sClassName, pszName));
    pObjects->push_back(std::unique_ptr<RndObject>(pAnim));
    pAnim->SetMesh(pMesh);
    AddRotationKeys(pMesh, pAnim, nKeys, flStartFrame, flStartAngle, flEndFrame, flEndAngle);
    if (bLoop) {
        auto *pLoop = new RndAnimatable::MinMaxLoop;
        pLoop->SetMin(flStartFrame);
        pLoop->SetMax(flEndFrame);
        pLoop->SetLoop(kLoopRotation);
        pAnim->GetFilters().push_back(pLoop);
    }
    pParent->AddAnim(pAnim);
}

char TnlGems::AddMeshGroups(const char *pszName, const char *pszDrawParent, float flLookahead) {
    const int nFirstType = static_cast<int>(mMeshGroups.size());
    RndDrawable *pParent = nullptr;
    if (pszDrawParent != nullptr) {
        pParent = dynamic_cast<RndDrawable *>(TheManager.Find(pszDrawParent));
    }
    MeshGroup *pPrev = nullptr;
    for (int n = 0;; ++n) {
        auto *pMultiMesh =
            dynamic_cast<RndMultiMesh *>(TheManager.Find(FormatString("%s%d.mm", pszName, n)));
        if (pMultiMesh == nullptr) {
            break;
        }
        if (pParent != nullptr) {
            pParent->AddDraw(pMultiMesh, nullptr);
        }
        SetRenderScale(pMultiMesh->GetMesh(), sMeshScale);
        mMeshGroups.push_back(std::make_unique<MeshGroup>(
            pMultiMesh, nullptr, nullptr, &mTransforms, mMaxGems, flLookahead));
        MeshGroup *pGroup = mMeshGroups.back().get();
        if (pPrev != nullptr) {
            pPrev->SetNext(pGroup);
        }
        pPrev = pGroup;
    }
    return static_cast<char>(nFirstType);
}

char TnlGems::AddCompositeMeshGroups(const char *pszName,
                                     const char *pszDrawParent,
                                     float flLookahead) {
    const int nFirstType = static_cast<int>(mMeshGroups.size());
    RndDrawable *pParent = nullptr;
    if (pszDrawParent != nullptr) {
        pParent = dynamic_cast<RndDrawable *>(TheManager.Find(pszDrawParent));
    }
    std::list<RndMesh *> meshes;
    MeshGroup *pPrev = nullptr;
    for (int n = 0;; ++n) {
        auto *pMultiMesh =
            dynamic_cast<RndMultiMesh *>(TheManager.Find(FormatString("%s_%d.mm", pszName, n)));
        if (pMultiMesh == nullptr) {
            break;
        }
        if (pParent != nullptr) {
            pParent->AddDraw(pMultiMesh, nullptr);
        }
        for (char nLetter = kFirstCompositeLetter;; ++nLetter) {
            auto *pMesh = dynamic_cast<RndMesh *>(
                TheManager.Find(FormatString("%s%c_%d.mesh", pszName, nLetter, n)));
            if (pMesh == nullptr) {
                break;
            }
            SetRenderScale(pMesh, sMeshScale);
            meshes.push_back(pMesh);
        }
        auto *pSys =
            dynamic_cast<RndParticleSys *>(TheManager.Find(FormatString("%s.ps", pszName)));
        if (pSys != nullptr) {
            SetRenderScale(static_cast<RndObject *>(pSys), sMeshScale);
        }
        mMeshGroups.push_back(std::make_unique<MeshGroup>(
            pMultiMesh, &meshes, pSys, &mTransforms, mMaxGems, flLookahead));
        MeshGroup *pGroup = mMeshGroups.back().get();
        if (pPrev != nullptr) {
            pPrev->SetNext(pGroup);
        }
        pPrev = pGroup;
        meshes.clear();
    }
    return static_cast<char>(nFirstType);
}

char TnlGems::AddSpriteGroup(const char *pszName) {
    const int nType = static_cast<int>(mSpriteGroups.size()) + TnlGem::kTypeSprite;
    mSpriteGroups.push_back(std::make_unique<SpriteGroup>(pszName, mMaxGems));
    RndParticleSys *pSys = mSpriteGroups.back()->GetParticles();
    if (pSys != nullptr) {
        SetRenderScale(static_cast<RndObject *>(pSys), sSpriteScale);
    }
    return static_cast<char>(nType);
}

void TnlGems::AddRotations(char nType,
                           int nKeys,
                           int bLoop,
                           RndAnimatable *pParent,
                           float flStartFrame,
                           float flStartAngle,
                           float flEndFrame,
                           float flEndAngle) {
    for (MeshGroup *pGroup = GetMeshGroup(nType); pGroup != nullptr; pGroup = pGroup->GetNext()) {
        if (pGroup->GetMeshes().empty()) {
            AddRotation(pGroup->GetMultiMesh()->GetMesh(),
                        &mObjects,
                        nKeys,
                        bLoop,
                        pParent,
                        flStartFrame,
                        flStartAngle,
                        flEndFrame,
                        flEndAngle);
        } else {
            for (auto *pMesh : pGroup->GetMeshes()) {
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
        auto *pMultiMesh =
            dynamic_cast<RndMultiMesh *>(TheManager.Find(FormatString("%s%d.mm", pszName, n)));
        if (pMultiMesh == nullptr) {
            break;
        }
        pMultiMesh->GetTransforms().clear();
    }
}

void TnlGems::ApplyRenderScales() {
    ApplyRenderScales(TheGfxManager.GetScrollSpeed());
}

MeshGroup *TnlGems::GetMeshGroup(char nType) {
    return mMeshGroups[nType].get();
}

SpriteGroup *TnlGems::GetSpriteGroup(char nType) {
    return mSpriteGroups[static_cast<char>(nType - TnlGem::kTypeSprite)].get();
}

void TnlGems::AddGem(
    char nTrack, char nType, char nSlot, int bFlash, char nColor, float flTick, float flShowTick) {
    auto it = FindNearest(flTick);
    for (; it != mGems.end() && it->GetTick() <= flTick; ++it) {
        if (flTick == it->GetTick() && nSlot == it->GetSlot() && nTrack == it->GetTrack()) {
            it->SetRemoveTick(flShowTick);
        }
    }
    TnlGem &gem = *mGems.insert(it, TnlGem());
    gem.SetType(nType);
    gem.SetTrack(nTrack);
    gem.SetTick(flTick);
    gem.SetSlot(nSlot);
    gem.SetFlags(bFlash ? TnlGem::kFlagFlash : TnlGem::kFlagNoFlash);
    gem.SetShowTick(flShowTick);
    if (nColor >= 0) {
        gem.SetFlags(gem.GetFlags() | nColor | TnlGem::kFlagPlayerColor);
    }
}

void TnlGems::RemoveGems(char nTrack, float flStartTick, float flEndTick) {
    auto it = FindNearest(flStartTick);
    while (it != mGems.end() && it->GetTick() < flEndTick) {
        if (it->GetTrack() == nTrack) {
            it->Release(mTunnel);
            it = mGems.erase(it);
        } else {
            ++it;
        }
    }
}

void TnlGems::RemoveGem(char nTrack, char nSlot, float flTick) {
    auto it = FindNearest(flTick);
    while (it != mGems.end() && it->GetTick() == flTick) {
        if (it->GetSlot() == nSlot && it->GetTrack() == nTrack) {
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
    const float flTick = TheGameDb->GetSongTick();
    const char nFocusTrack = mHasFocusTrack ? mFocusTrack : kNoTrack;
    const float flTrailTick = flTick - mTrailTicks;
    auto it = mGems.begin();
    while (it != mGems.end() && it->GetTick() < flTrailTick) {
        it->Release(nullptr);
        it = mGems.erase(it);
    }

    const float flHorizonTick = flTick + kHorizonTicks;
    int nPlaced = 0;
    while (it != mGems.end()) {
        if (it->GetRemoveTick() < flTick || flHorizonTick < it->GetTick()) {
            it->Release(mTunnel);
            it = mGems.erase(it);
            continue;
        }
        if ((it->GetFlags() & TnlGem::kFlagPassed) == 0 && kPassTicks < flTick - it->GetTick()) {
            if ((it->GetFlags() & TnlGem::kFlagFlash) != 0) {
                it->Flash(mTunnel);
            }
            it->SetFlags(it->GetFlags() | TnlGem::kFlagPassed);
            TheTriggerMgr.GemEvent(it->GetTrack(), it->GetSlot());
            if (it->GetTrack() == nFocusTrack) {
                TheTriggerMgr.HitEvent(kFocusPlayer, it->GetSlot());
            }
        }
        if (nPlaced < mMaxGems) {
            if (it->GetShowTick() < flTick) {
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
    for (const auto &pGroup : mMeshGroups) {
        if (pGroup->GetMeshes().empty()) {
            continue;
        }
        RndMultiMesh *pMultiMesh = pGroup->GetMultiMesh();
        auto it = pGroup->GetMeshes().begin();
        pMultiMesh->SetMesh(*it);
        pMultiMesh->Draw();
        ++it;
        if (pGroup->GetParticles() != nullptr) {
            pGroup->GetParticles()->Draw();
        }
        for (; it != pGroup->GetMeshes().end(); ++it) {
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
    if (it->GetTick() < flTick) {
        return mGems.end();
    }
    if (std::fabs(it->GetTick() - flTick) < std::fabs(first->GetTick() - flTick)) {
        while (it != first) {
            const auto prev = std::prev(it);
            if (prev->GetTick() < flTick) {
                return it;
            }
            it = prev;
        }
        return first;
    }
    for (; first != mGems.end(); ++first) {
        if (flTick <= first->GetTick()) {
            break;
        }
    }
    return first;
}

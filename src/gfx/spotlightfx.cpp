#include "gfx/spotlightfx.h"

#include <algorithm>
#include <iterator>

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "math/color.h"
#include "math/interpolator.h"
#include "math/vector2.h"
#include "rnd/animatable.h"
#include "rnd/environ.h"
#include "rnd/manager.h"

namespace {

// The pool starts with this many transforms.
constexpr int kInitialTransforms = 96;
// A light this many ticks behind the song is dropped.
constexpr float kTrailTicks = 960.0f;

// NTSC-U/C: 0x003afb98
float sFadeEndTicks = 960.0f;
// NTSC-U/C: 0x003afb9c
float sFadeStartTicks = 480.0f;
// The scale of the spotlight transforms on each axis.
// NTSC-U/C: 0x0043b620
Vector3 sSpotScale{1.0f, 1.0f, 1.0f};
// The scale of the glow mesh on each axis.
// NTSC-U/C: 0x0043b630
Vector3 sGlowScale{1.0f, 1.0f, 1.0f};

// Set the local transform of a transformable and mark it dirty.
inline void SetLocalXfm(Rnd::Transformable *pTrans, const Transform &xfm) {
    const Vector3 *rows[] = {&xfm.mBasisX, &xfm.mBasisY, &xfm.mBasisZ, &xfm.mTranslation};
    for (int i = 0; i < Rnd::kXfmRowCount; ++i) {
        pTrans->mLocalXfm[i][0] = rows[i]->x;
        pTrans->mLocalXfm[i][1] = rows[i]->y;
        pTrans->mLocalXfm[i][2] = rows[i]->z;
        pTrans->mLocalXfm[i][kVec3PaddingFloat] = rows[i]->w;
    }
    pTrans->mDirty = 1;
}

} // namespace

SpotLightFX::SpotLightFX() {
    mMesh = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find("gem spotlight.mesh"));
    auto *pGlowMesh = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find("gem base glow.mesh"));
    mSpots = dynamic_cast<Rnd::MultiMesh *>(
        Rnd::TheManager.Create(Rnd::g_multiMeshClassName.mStr, "gem spotlight.mm"));
    mSpots->SetMesh(mMesh);
    mGlows = dynamic_cast<Rnd::MultiMesh *>(
        Rnd::TheManager.Create(Rnd::g_multiMeshClassName.mStr, "gem base glow.mm"));
    mGlows->SetMesh(pGlowMesh);
    auto *pSpotAnim = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("gem spotlight.mnm"));
    dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("tnl.view"))->AddAnim(pSpotAnim);

    Transform scale;
    scale.mBasisX = Vector3{sGlowScale.x, 0.0f, 0.0f};
    scale.mBasisY = Vector3{0.0f, sGlowScale.y, 0.0f};
    scale.mBasisZ = Vector3{0.0f, 0.0f, sGlowScale.z};
    scale.mTranslation = Vector3{0.0f, 0.0f, 0.0f};
    pGlowMesh->TransformVerts(scale);
    auto *pGlowAnim = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("gem base glow.mnm"));
    dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("realtime.view"))->AddAnim(pGlowAnim);
    // The original fills the pool from an uninitialised temporary.
    mTransforms.resize(kInitialTransforms, Transform());
}

SpotLightFX::~SpotLightFX() {
    delete mGlows;
    delete mSpots;
}

void SpotLightFX::LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *, int) {
    Vector2 fade;
    FindConfigVector2(pConfig, pDefaults, "spotlight_fade_ticks", &fade, true);
    sFadeStartTicks = (fade.y < fade.x) ? fade.y : fade.x;
    sFadeEndTicks = (fade.x < fade.y) ? fade.y : fade.x;
    FindConfigVector3(pConfig, pDefaults, "spotlight_scale", &sSpotScale, true);
    float flBaseScale;
    FindConfigFloat(pConfig, pDefaults, "spotlight_base_scale", &flBaseScale, true);
    const float flWorldScale = TheGfxManager.mScrollSpeed;
    sGlowScale.x = flBaseScale / sSpotScale.x;
    sGlowScale.y = flBaseScale / sSpotScale.y;
    sGlowScale.z = flBaseScale / sSpotScale.z;
    sSpotScale.x *= flWorldScale;
    sSpotScale.y *= flWorldScale;
    sSpotScale.z *= flWorldScale;
}

void SpotLightFX::Poll() {
}

void SpotLightFX::Draw() {
    const float flTick = TheGameDb->mSongTick;
    const float flFadeStart = flTick + sFadeStartTicks;
    const float flFadeEnd = flTick + sFadeEndTicks;
    const float flTrailTick = flTick - kTrailTicks;
    LinearInterpolator fade(0.0f, 1.0f, flFadeStart, flFadeEnd);

    auto it = mLights.begin();
    while (it != mLights.end() && !(flTrailTick < it->mTick)) {
        Release(it);
        it = mLights.erase(it);
    }
    mGlows->Draw();

    Rnd::Environ *pEnviron = Rnd::Environ::sCurrent;
    const Color ambient = pEnviron->mAmbient;
    for (it = mLights.begin(); it != mLights.end() && it->mTick < flFadeEnd; ++it) {
        if (it->mSpotActive) {
            DetachSpot(it);
        }
        if (flFadeStart < it->mTick) {
            const float flLevel = fade.LinearInterpolator::Interp(it->mTick);
            pEnviron->mAmbient = Color{flLevel, flLevel, flLevel, 1.0f};
            pEnviron->Draw();
            SetLocalXfm(mMesh, *it->mSpot);
            mMesh->UpdateWorldXfm(nullptr, 0);
            mMesh->Draw();
        }
    }
    pEnviron->mAmbient = Color{1.0f, 1.0f, 1.0f, 1.0f};
    pEnviron->Draw();
    mSpots->Draw();
    pEnviron->mAmbient = ambient;
    pEnviron->Draw();
}

void SpotLightFX::AddLight(char nTrack, TnlGeom *pGeom, float flTick, float flLateral) {
    if (mTransforms.empty() || std::next(mTransforms.begin()) == mTransforms.end()) {
        // The original appends transforms from an uninitialised temporary.
        mTransforms.push_back(Transform());
        mTransforms.push_back(Transform());
    }
    auto it = std::lower_bound(mLights.begin(), mLights.end(), flTick, LightPos::Before);
    if (it != mLights.end() && it->mTick == flTick && it->mTrack == nTrack) {
        return;
    }
    LightPos light;
    light.mSpotActive = 0;
    it = mLights.insert(it, light);
    it->mTick = flTick;
    it->mTrack = nTrack;
    it->mLateral = flLateral;
    std::list<Transform> &glows = mGlows->GetTransforms();
    std::list<Transform> &spots = mSpots->GetTransforms();
    glows.splice(glows.begin(), mTransforms, mTransforms.begin());
    spots.splice(spots.begin(), mTransforms, mTransforms.begin());
    it->mGlow = glows.begin();
    it->mSpot = spots.begin();
    it->mSpotActive = 1;
    Place(&*it, pGeom);
}

void SpotLightFX::RemoveLight(char nTrack, float flTick) {
    RemoveLights(nTrack, flTick, flTick + 1.0f);
}

void SpotLightFX::RemoveLights(char nTrack, float flStartTick, float flEndTick) {
    auto it = std::lower_bound(mLights.begin(), mLights.end(), flStartTick, LightPos::Before);
    while (it != mLights.end() && it->mTick < flEndTick) {
        if (it->mTrack == nTrack) {
            Release(it);
            it = mLights.erase(it);
        } else {
            ++it;
        }
    }
}

void SpotLightFX::Clear() {
    auto it = mLights.begin();
    while (it != mLights.end()) {
        Release(it);
        it = mLights.erase(it);
    }
}

void SpotLightFX::UpdateRange(TnlGeom *pGeom, const TnlTrackRange *pRange) {
    auto it =
        std::lower_bound(mLights.begin(), mLights.end(), pRange->mStartTick, LightPos::Before);
    for (; it != mLights.end() && it->mTick < pRange->mEndTick; ++it) {
        if (pRange->mFirstTrack <= it->mTrack && it->mTrack < pRange->mEndTrack) {
            Place(&*it, pGeom);
        }
    }
}

void SpotLightFX::Release(std::list<LightPos>::iterator light) {
    mTransforms.splice(mTransforms.end(), mGlows->GetTransforms(), light->mGlow);
    if (light->mSpotActive) {
        mTransforms.splice(mTransforms.end(), mSpots->GetTransforms(), light->mSpot);
        light->mSpot = std::prev(mTransforms.end());
        light->mSpotActive = 0;
    }
}

void SpotLightFX::DetachSpot(std::list<LightPos>::iterator light) {
    if (light->mSpotActive) {
        mTransforms.splice(mTransforms.end(), mSpots->GetTransforms(), light->mSpot);
        light->mSpot = std::prev(mTransforms.end());
        light->mSpotActive = 0;
    }
}

void SpotLightFX::Place(LightPos *pLight, TnlGeom *pGeom) {
    Transform &glow = *pLight->mGlow;
    pGeom->CellXfm(pLight->mTrack, &glow, false, true, pLight->mTick, pLight->mLateral);
    glow.mBasisX.x *= sSpotScale.x;
    glow.mBasisX.y *= sSpotScale.x;
    glow.mBasisX.z *= sSpotScale.x;
    glow.mBasisY.x *= sSpotScale.y;
    glow.mBasisY.y *= sSpotScale.y;
    glow.mBasisY.z *= sSpotScale.y;
    glow.mBasisZ.x *= sSpotScale.z;
    glow.mBasisZ.y *= sSpotScale.z;
    glow.mBasisZ.z *= sSpotScale.z;
    *pLight->mSpot = glow;
}

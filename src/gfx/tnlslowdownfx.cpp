#include "gfx/tnlslowdownfx.h"

#include <math.h>

#include "game/gamedb.h"
#include "rnd/manager.h"
#include "rnd/rndrenderer.h"

namespace {

// The components of a row of a transform, and the row of the translation.
constexpr int kX = 0;
constexpr int kY = 1;
constexpr int kZ = 2;
constexpr int kTranslationRow = Rnd::kXfmRowCount - 1;

// The scale of the overlay beyond the diagonal of the screen.
constexpr float kOverscan = 1.05f;

// The position of the mesh across and along the screen, the second as a share of the aspect ratio.
constexpr float kMeshX = 0.5f;
constexpr float kMeshZPerAspect = -0.5f;

// The frames the animation of the material runs through over a slowdown.
constexpr float kAnimFrames = 1000.0f;

// The input range of a stopped animation, ticks the song never arrives at.
constexpr float kStoppedStartTick = -1e9f;
constexpr float kStoppedEndTick = -999990016.0f;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

} // namespace

TnlSlowdownFX::TnlSlowdownFX(const char *pszName, GfxTunnel *pTunnel)
    : mTunnel(pTunnel), mAnim(nullptr), mFrames(0.0f, 0.0f, 0.0f, 1.0f), mTransAnim(nullptr),
      mMesh(nullptr), mName(pszName) {
    mAnim = FindObject<Rnd::Animatable>("slowdown overlay.mnm");
    mTransAnim = FindObject<Rnd::TransAnim>("slowdown overlay.tnm");
    mMesh = FindObject<Rnd::Mesh>("slowdown overlay.mesh");
    const float flAspect = TheRnd->AspectRatio();
    const float flScale = sqrtf((flAspect * flAspect) + 1.0f) * kOverscan;
    Rnd::TransAnim::TransKey &key = mTransAnim->GetFramesOwner()->mScaleKeys.front();
    key.mValue[kZ] = flScale;
    key.mValue[kX] = flScale;
    key.mValue[kY] = flScale;
    mMesh->mDirty = 1;
    mMesh->mLocalXfm[kTranslationRow][kX] = kMeshX;
    mMesh->mLocalXfm[kTranslationRow][kY] = 0.0f;
    mMesh->mLocalXfm[kTranslationRow][kZ] = flAspect * kMeshZPerAspect;
    Reset();
}

TnlSlowdownFX::~TnlSlowdownFX() {
}

void TnlSlowdownFX::LoadConfig(DataArray *, DataArray *, GfxTunnel *, int) {
}

void TnlSlowdownFX::Reset() {
    mMesh->SetShowing(0);
    mFrames.Reset(0.0f, 0.0f, kStoppedStartTick, kStoppedEndTick);
}

void TnlSlowdownFX::Start(float, float, float flStopTick) {
    mFrames.Reset(0.0f, kAnimFrames, TheGameDb->mSongTick, flStopTick);
    mMesh->SetShowing(1);
}

void TnlSlowdownFX::Update() {
    const float flTick = TheGameDb->mSongTick;
    if (mFrames.mX1 <= flTick) {
        return;
    }
    mAnim->SetFrame(mFrames.Interp(flTick));
    mTransAnim->SetFrame(flTick);
    mMesh->UpdateWorldXfm(nullptr, 0);
}

void TnlSlowdownFX::Draw() {
    mMesh->Draw();
}

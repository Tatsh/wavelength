#include "gfx/tnlslowdownfx.h"

#include <math.h>

#include "game/gamedb.h"
#include "math/key.h"
#include "rnd/rndmanager.h"
#include "rnd/rndrenderer.h"

namespace {

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
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

} // namespace

TnlSlowdownFX::TnlSlowdownFX(const char *pszName, GfxTunnel *pTunnel)
    : mTunnel(pTunnel), mAnim(nullptr), mFrames(0.0f, 0.0f, 0.0f, 1.0f), mTransAnim(nullptr),
      mMesh(nullptr), mName(pszName) {
    mAnim = FindObject<RndAnimatable>("slowdown overlay.mnm");
    mTransAnim = FindObject<RndTransAnim>("slowdown overlay.tnm");
    mMesh = FindObject<RndMesh>("slowdown overlay.mesh");
    const float flAspect = TheRnd->AspectRatio();
    const float flScale = sqrtf((flAspect * flAspect) + 1.0f) * kOverscan;
    Key<Vector3> &key = mTransAnim->mFramesOwner->mScaleKeys.front();
    key.value.z = flScale;
    key.value.x = flScale;
    key.value.y = flScale;
    mMesh->mDirty = 1;
    mMesh->mLocalXfm.mTranslation.x = kMeshX;
    mMesh->mLocalXfm.mTranslation.y = 0.0f;
    mMesh->mLocalXfm.mTranslation.z = flAspect * kMeshZPerAspect;
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
    mFrames.Reset(0.0f, kAnimFrames, TheGameDb->GetSongTick(), flStopTick);
    mMesh->SetShowing(1);
}

void TnlSlowdownFX::Update() {
    const float flTick = TheGameDb->GetSongTick();
    if (mFrames.GetX1() <= flTick) {
        return;
    }
    mAnim->SetFrame(mFrames.Interp(flTick));
    mTransAnim->SetFrame(flTick);
    mMesh->UpdateWorldXfm(nullptr, 0);
}

void TnlSlowdownFX::Draw() {
    mMesh->Draw();
}

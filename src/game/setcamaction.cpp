#include "game/setcamaction.h"

namespace {

constexpr float kPi = 3.14159265f;
constexpr float kDegreesPerHalfTurn = 180.0f;

} // namespace

SetCamAction::SetCamAction(DataArray *pAction) {
    mCam = FindObject<Rnd::Cam>(pAction, pAction->Sym(1));
    mFlags = 0;
    if (pAction->FindFloat("near", &mNear, false)) {
        mFlags |= kFlagNear;
    }
    if (pAction->FindFloat("far", &mFar, false)) {
        mFlags |= kFlagFar;
    }
    if (pAction->FindFloat("fov", &mFov, false)) {
        mFlags |= kFlagFov;
        mFov = (mFov * kPi) / kDegreesPerHalfTurn;
    }
}

void SetCamAction::Exec() {
    if (mFlags == 0) {
        return;
    }
    const float flNear = (mFlags & kFlagNear) != 0 ? mNear : mCam->GetNearPlane();
    const float flFar = (mFlags & kFlagFar) != 0 ? mFar : mCam->GetFarPlane();
    const float flFov = (mFlags & kFlagFov) != 0 ? mFov : mCam->GetFov();
    mCam->SetFrustum(flNear, flFar, flFov);
}

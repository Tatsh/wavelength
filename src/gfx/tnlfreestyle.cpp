#include "gfx/tnlfreestyle.h"

#include <cmath>

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "gfx/playercamfx.h"
#include "gfx/tnlgeom.h"
#include "math/transformops.h"

namespace {

// A change takes this many ticks instead of the configured delay when it is quick.
constexpr float kQuickTicks = 4.0f;

// The beams of the freestylus grow to this pose while the player plays.
constexpr float kBeamsPlaying = 120.0f;

// A sample starts the freestylus at this pose.
constexpr float kSampleStart = 0.0f;
constexpr float kSampleEnd = 500.0f;

// The pitch and the colour of a freestylus that does not follow the stick.
constexpr float kNeutralPose = 0.5f;

// `tumble_angle_per_ms` is in degrees, and a half turn is this many radians and degrees.
constexpr float kPi = 3.1415927f;
constexpr float kHalfTurnDegrees = 180.0f;

// The row of a transform that holds the translation.
constexpr int kTranslationRow = 3;

// The rows of a basis.
constexpr int kBasisRows = 3;

// Give an approach the curve and the greatest speed of its configuration.
inline void SetCurve(QuadraticApproach *pApproach, const Vector3 &curve, float flMaxSpeed) {
    pApproach->mSquare = curve.x;
    pApproach->mLinear = curve.y;
    pApproach->mConstant = curve.z;
    pApproach->mMaxSpeed = flMaxSpeed;
}

} // namespace

// The static initialiser of the unit, and the routine that runs it to construct the statics of
// the freestyle.
// NTSC-U/C: 0x001e9d70, PAL: 0x001f2b10
// NTSC-U/C: 0x001e9f78, PAL: 0x001f2d18

float TnlFreestyle::sEnterDelay = 480.0f;
float TnlFreestyle::sExitDelay = 480.0f;
float TnlFreestyle::sMaxChangePerMs = 0.01f;
float TnlFreestyle::sCamMaxChangePerMs = 0.01f;
float TnlFreestyle::sLaneScale = 2.0f;
float TnlFreestyle::sInvLaneScale = 0.5f;
Freestylus *TnlFreestyle::sFreestylus = nullptr;
Vector2 TnlFreestyle::sRange{4.0f, 2.0f};
Vector2 TnlFreestyle::sInvRange{0.25f, 0.5f};
// Yes, the fourth word of these vectors is zero because the binary never writes it.
Vector3 TnlFreestyle::sCenter{0.0f, 0.0f, 2.0f, 0.0f};
Vector3 TnlFreestyle::sConstrainedCenter{0.0f, 0.0f, 2.0f, 0.0f};
Vector3 TnlFreestyle::sCamOffset{0.0f, -4.0f, 3.0f, 0.0f};
Vector3 TnlFreestyle::sCamLookOffset{0.0f, 0.0f, 1.0f, 0.0f};
Vector3 TnlFreestyle::sCamWinOffset{0.0f, 0.0f, 1.0f, 0.0f};
Vector3 TnlFreestyle::sCamWinLookOffset{0.0f, 0.0f, 0.0f, 0.0f};
Vector2 TnlFreestyle::sCamRange{0.7f, 0.7f};
Vector2 TnlFreestyle::sConstrainedCamRange{0.7f, 0.7f};
Vector3 TnlFreestyle::sChangeFunc{0.1f, 0.0f, 0.0f, 0.0f};
Vector3 TnlFreestyle::sTumbleChangeFunc{0.1f, 0.0f, 0.0f, 0.0f};
Vector3 TnlFreestyle::sCamChangeFunc{0.1f, 0.0f, 0.0f, 0.0f};
Vector3 TnlFreestyle::sConstraintChangeFunc{0.1f, 0.0f, 0.0f, 0.0f};

TnlFreestyle::TnlFreestyle(
    int nPlayer, int nType, Ship *pShip, TnlActivator *pActivator, GfxTunnel *pTunnel)
    : mTrail(new TnlSabreTrail(nPlayer, pShip)), mPlayer(nPlayer), mActive(0), mAssembled(0),
      mPlaying(0), mType(nType), mTunnel(pTunnel), mShip(pShip), mActivator(pActivator),
      mAssembly(new LinearInterpolator(0.0f, 0.0f, 0.0f, 1.0f)),
      mCamBlend(new LinearInterpolator(0.0f, 0.0f, 0.0f, 1.0f)),
      mX(sChangeFunc, 0.0f, sMaxChangePerMs), mY(sChangeFunc, 0.0f, sMaxChangePerMs),
      mTumbleX(sTumbleChangeFunc, 0.0f, 1.0f), mTumbleY(sTumbleChangeFunc, 0.0f, 1.0f),
      mCamX(sCamChangeFunc, 0.0f, sCamMaxChangePerMs),
      mCamY(sCamChangeFunc, 0.0f, sCamMaxChangePerMs),
      mConstraint(sConstraintChangeFunc, 0.0f, 1.0f) {
    mStick.x = 0.0f;
    mStick.z = 0.0f;
    mStick.y = 0.0f;
    mShipPos.x = 0.0f;
    mShipPos.z = 0.0f;
    mShipPos.y = 0.0f;
    if (sFreestylus == nullptr) {
        sFreestylus = new Freestylus(mTunnel, mType);
    }
}

TnlFreestyle::~TnlFreestyle() {
    delete sFreestylus;
    sFreestylus = nullptr;
    delete mTrail;
    delete mCamBlend;
    delete mAssembly;
}

void TnlFreestyle::LoadConfig(
    DataArray *pConfig, DataArray *pDefaults, GfxTunnel *, int nOption, int bFreestyle) {
    mTrail->LoadConfig(pConfig, pDefaults);
    if (bFreestyle) {
        DataArray *pSection;
        FindConfigArray(pConfig, pDefaults, "freestyle", &pSection, true);
        DataArray *pType =
            pSection->FindArray((mType == Freestylus::kTypeAxe) ? "axe" : "scratch", false);
        FindConfigFloat(pType, pSection, "enter_delay", &sEnterDelay, true);
        FindConfigFloat(pType, pSection, "exit_delay", &sExitDelay, true);
        FindConfigVector2(pType, pSection, "range", &sRange, true);
        sInvRange.x = 1.0f / sRange.x;
        sInvRange.y = 1.0f / sRange.y;
        FindConfigVector3(pType, pSection, "center", &sCenter, true);
        FindConfigVector3(pType, pSection, "constrained_center", &sConstrainedCenter, true);
        FindConfigVector3(pType, pSection, "cam_offset", &sCamOffset, true);
        FindConfigVector3(pType, pSection, "cam_look_offset", &sCamLookOffset, true);
        FindConfigVector3(pType, pSection, "cam_win_offset", &sCamWinOffset, true);
        FindConfigVector3(pType, pSection, "cam_win_look_offset", &sCamWinLookOffset, true);
        const float flScale = TheGfxManager.mScrollSpeed;
        sCenter.x *= flScale;
        sCenter.z *= flScale;
        sConstrainedCenter.x *= flScale;
        sConstrainedCenter.z *= flScale;
        sCenter.y *= flScale;
        sConstrainedCenter.y *= flScale;
        FindConfigVector2(pType, pSection, "cam_range", &sCamRange, true);
        FindConfigVector2(pType, pSection, "constrained_cam_range", &sConstrainedCamRange, true);
        FindConfigFloat(pType, pSection, "cam_max_change_per_ms", &sCamMaxChangePerMs, true);
        FindConfigVector3(pType, pSection, "cam_change_func", &sCamChangeFunc, true);
        FindConfigFloat(pType, pSection, "max_change_per_ms", &sMaxChangePerMs, true);
        FindConfigVector3(pType, pSection, "change_func", &sChangeFunc, true);
        FindConfigVector3(pType, pSection, "tumble_change_func", &sTumbleChangeFunc, true);
        if (FindConfigFloat(
                pType, pSection, "tumble_angle_per_ms", &Freestylus::sTumbleAnglePerMs, true)) {
            Freestylus::sTumbleAnglePerMs =
                (Freestylus::sTumbleAnglePerMs * kPi) / kHalfTurnDegrees;
        }
        FindConfigVector3(pType, pSection, "constraint_change_func", &sConstraintChangeFunc, true);
        if (!nOption) {
            float flFreestylerScale = 1.0f;
            FindConfigFloat(pType, pSection, "freestyler_scale", &flFreestylerScale, false);
            sFreestylus->Setup(flFreestylerScale);
        }
    }
    sLaneScale = (TheGameDb->mCommunity == GameDb::kCommunityLocal) ? 1.0f : 2.0f;
    sInvLaneScale = 1.0f / sLaneScale;
    SetCurve(&mX, sChangeFunc, sMaxChangePerMs);
    SetCurve(&mY, sChangeFunc, sMaxChangePerMs);
    SetCurve(&mTumbleX, sTumbleChangeFunc, 1.0f);
    SetCurve(&mTumbleY, sTumbleChangeFunc, 1.0f);
    SetCurve(&mCamX, sCamChangeFunc, sCamMaxChangePerMs);
    SetCurve(&mCamY, sCamChangeFunc, sCamMaxChangePerMs);
    SetCurve(&mConstraint, sConstraintChangeFunc, 1.0f);
}

void TnlFreestyle::Reset() {
    if (mActive && !mAssembled) {
        if (mAssembly->mY1 == 1.0f) {
            OnAssembled();
            mAssembled = 1;
        } else {
            EndPass();
            mActive = 0;
        }
    }
    sFreestylus->Reset();
    mTrail->Clear();
}

void TnlFreestyle::SetActive(bool bActive, int bVictory, bool bQuick) {
    const float flTick = TheGameDb->mSongTick;
    if (!bActive) {
        if (!mActive) {
            return;
        }
        float flAssembly = 1.0f;
        float flCam = 1.0f;
        if (!mAssembled) {
            if (mAssembly->mY1 == 0.0f) {
                return;
            }
            flAssembly = mAssembly->Interp(flTick);
            flCam = mCamBlend->Interp(flTick);
        }
        const float flEndTick = flTick + (bQuick ? kQuickTicks : sExitDelay);
        mAssembly->Reset(flAssembly, 0.0f, flTick, flEndTick);
        mCamBlend->Reset(flCam, 0.0f, flTick, flEndTick);
        OnDisassemble();
        mAssembled = 0;
        return;
    }

    float flAssembly = 0.0f;
    float flCam = 0.0f;
    mStartTick = flTick;
    if (mActive) {
        if (mVictory != bVictory) {
            if (mAssembled) {
                OnDisassemble();
            }
            EndPass();
            mActive = 0;
        } else if (mAssembled || (mAssembly->mY1 == 1.0f)) {
            return;
        }
        flAssembly = mAssembly->Interp(flTick);
        flCam = mCamBlend->Interp(flTick);
    } else {
        mStick.x = 0.0f;
        mStick.z = 0.0f;
        mStick.y = 0.0f;
    }
    mVictory = bVictory;
    const float flEndTick = flTick + (bQuick ? kQuickTicks : sEnterDelay);
    mAssembly->Reset(flAssembly, 1.0f, flTick, flEndTick);
    mCamBlend->Reset(flCam, 1.0f, flTick, flEndTick);
    mAssembled = 0;
    mActive = 1;
    sFreestylus->mBeamGrow.SetTarget(0.0f);
    BeginPass();
}

void TnlFreestyle::SetPlaying(bool bPlaying, [[maybe_unused]] int nColumn) {
    if (!mActive) {
        return;
    }
    mPlaying = bPlaying;
    const bool bGrow = bPlaying && (mType != Freestylus::kTypeScratch);
    sFreestylus->mBeamGrow.SetTarget(bGrow ? kBeamsPlaying : 0.0f);
    if (mType != Freestylus::kTypeScratch) {
        return;
    }
    if (bPlaying) {
        mTrail->Start();
    } else {
        mTrail->Stop(false);
    }
}

void TnlFreestyle::Sample() {
    if (!mActive) {
        return;
    }
    sFreestylus->mSample.Jump(kSampleStart, kSampleEnd);
    if (mType == Freestylus::kTypeScratch) {
        mTrail->StartSample();
    }
}

void TnlFreestyle::SetStick(const Vector3 &stick) {
    mStick = stick;
}

void TnlFreestyle::Scratch(float flTick, float flDuration) {
    if (mType == Freestylus::kTypeScratch) {
        mTrail->Scratch(flTick, flDuration);
    }
}

void TnlFreestyle::Poll(float flTicks, float flTime) {
    const float flTick = TheGameDb->mSongTick;
    if (!mActive) {
        return;
    }
    int bCam;
    int bShip;
    int bShowShip;
    GetPassFlags(&bCam, &bShip, &bShowShip);
    float flCam = 1.0f;
    const float flAbsTime = std::fabs(flTime);
    const float flAbsTicks = std::fabs(flTicks);
    float flAssembly = flCam;
    Vector3 stick;
    if (mAssembled) {
        stick = mStick;
    } else {
        flAssembly = mAssembly->Interp(flTick);
        flCam = mCamBlend->Interp(flTick);
        const bool bAssemblyDone = mAssembly->mX1 <= flTick;
        const bool bCamDone = mCamBlend->mX1 <= flTick;
        if (mAssembly->mY1 == 1.0f) {
            if (bCamDone) {
                flCam = 1.0f;
                if (bAssemblyDone) {
                    flAssembly = flCam;
                    OnAssembled();
                    mAssembled = 1;
                }
            } else if (bAssemblyDone) {
                flAssembly = 1.0f;
            }
        } else if (bCamDone) {
            flCam = 0.0f;
            if (bAssemblyDone) {
                EndPass();
                mActive = 0;
                return;
            }
        } else if (bAssemblyDone) {
            flAssembly = 0.0f;
        }
        stick.x = mStick.x * flAssembly;
        stick.z = mStick.z * flAssembly;
        stick.y = mStick.y * flAssembly;
    }
    const bool bPlaying = mPlaying && (mType != Freestylus::kTypeScratch);
    sFreestylus->Update(bPlaying, flTicks, flTime, flAssembly);
    RndTransAnim *pAnim = (mVictory && bShip) ? Ship::sWinFlyOff : nullptr;
    float shipXfm[kTransformRowCount][kTransformRowFloatCount];
    Transform camXfm;
    ComputeXfms(
        bCam != 0, pAnim, &stick, shipXfm, &camXfm, flAbsTime, flAbsTicks, flTick - mStartTick);
    if (mType == Freestylus::kTypeScratch) {
        const float flNeedle = mTrail->UpdateSample();
        for (int i = 0; i < kBasisRows; ++i) {
            shipXfm[kTranslationRow][i] += shipXfm[0][i] * flNeedle;
        }
    }
    if (bShip) {
        mShip->SetOverride(shipXfm, flAssembly);
    }
    if (bCam) {
        mTunnel->mCamFX->SetSlide(&camXfm, flCam);
    }
    float flPitch;
    float flColor;
    if (mType == Freestylus::kTypeAxe) {
        flPitch = (((sInvRange.x * kNeutralPose) * mX.mValue) + kNeutralPose);
        flColor = (((sInvRange.y * kNeutralPose) * mY.mValue) + kNeutralPose);
    } else {
        flColor = kNeutralPose;
        flPitch = kNeutralPose;
    }
    sFreestylus->SetPitchColor(flPitch, flColor);
}

void TnlFreestyle::UpdateTrail() {
    if (mType != Freestylus::kTypeScratch) {
        return;
    }
    float flBlend = mY.mValue * sInvRange.y;
    if (0.0f < flBlend) {
        flBlend *= sInvLaneScale;
    }
    if (1.0f < flBlend) {
        flBlend = 1.0f;
    } else if (flBlend < -1.0f) {
        flBlend = -1.0f;
    }
    mTrail->Update(flBlend, static_cast<RndTransformable *>(mShip->mView)->mWorldXfm, &mPathAxis);
}

void TnlFreestyle::Draw() {
    if (!mActive || mVictory) {
        return;
    }
    sFreestylus->Draw(mShip->mView, mAssembled);
    if (mType == Freestylus::kTypeScratch) {
        mTrail->mRibbon->Draw();
    }
}

void TnlFreestyle::ComputeXfms(bool bCam,
                               RndTransAnim *pAnim,
                               const Vector3 *pStick,
                               float (*pShipXfm)[kTransformRowFloatCount],
                               Transform *pCamXfm,
                               float flTime,
                               float,
                               float flElapsed) {
    float flStickY = pStick->z * sRange.y;
    const float flStickX = pStick->x * sRange.x;
    if (0.0f < flStickY) {
        flStickY *= sLaneScale;
    }
    float flConstraint = 0.0f;
    (void)mConstraint.Step(flTime, mTunnel->GetFreestyleConstraint());
    if (bCam) {
        flConstraint = mConstraint.mValue;
    }
    const float flCamRangeX = ((sConstrainedCamRange.x - sCamRange.x) * flConstraint) + sCamRange.x;
    const float flSpeedX = mX.Step(flTime, flStickX);
    const float flSpeedY = mY.Step(flTime, flStickY);
    (void)mTumbleX.Step(flTime, flSpeedX);
    (void)mTumbleY.Step(flTime, flSpeedY);
    sFreestylus->Spin(flTime, mTumbleX.mValue, mTumbleY.mValue);

    for (int i = 0; i < kBasisRows; ++i) {
        for (int j = 0; j < kBasisRows; ++j) {
            pShipXfm[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
    pShipXfm[kTranslationRow][0] = mX.mValue;
    pShipXfm[kTranslationRow][2] = mY.mValue;
    pShipXfm[kTranslationRow][1] = 0.0f;

    Vector3 center;
    if (flConstraint == 0.0f) {
        center = sCenter;
    } else if (flConstraint == 1.0f) {
        center = sConstrainedCenter;
    } else {
        const float flRest = 1.0f - flConstraint;
        center = Vector3{(sConstrainedCenter.x * flConstraint) + (sCenter.x * flRest),
                         (sConstrainedCenter.y * flConstraint) + (sCenter.y * flRest),
                         (sConstrainedCenter.z * flConstraint) + (sCenter.z * flRest),
                         sConstrainedCenter.w};
    }

    if (bCam) {
        (void)mCamX.Step(flTime, mX.mValue);
        (void)mCamY.Step(flTime, mY.mValue);
        const float flCamX = flCamRangeX * mCamX.mValue;
        const float flCamY = mCamY.mValue;
        Vector3 &camPos = pCamXfm->mTranslation;
        camPos.y = 0.0f;
        camPos.x = flCamX;
        camPos.z = ((1.0f - flConstraint) * sCamRange.y) * flCamY;
        if (mVictory) {
            const Vector3 look{pShipXfm[kTranslationRow][0] + sCamWinLookOffset.x,
                               pShipXfm[kTranslationRow][1] + sCamWinLookOffset.y,
                               pShipXfm[kTranslationRow][2] + sCamWinLookOffset.z};
            const Vector3 dir{look.x - flCamX, look.y - camPos.y, look.z - camPos.z};
            camPos.x = flCamX + sCamWinOffset.x;
            camPos.y += sCamWinOffset.y;
            camPos.z += sCamWinOffset.z;
            const Vector3 up{0.0f, 0.0f, 1.0f};
            Mat33BuildOrthonormal(&dir.x, &up.x, &pCamXfm->mBasisX.x);
        } else {
            const float flLift = ((sConstrainedCamRange.y * flConstraint) * flCamY);
            const Vector3 dir{sCamLookOffset.x + 0.0f,
                              sCamLookOffset.y + 1.0f,
                              (sCamLookOffset.z + camPos.y) + flLift};
            const Vector3 up{camPos.y, camPos.y, 1.0f};
            Mat33BuildOrthonormal(&dir.x, &up.x, &pCamXfm->mBasisX.x);
            camPos.x += sCamOffset.x;
            camPos.y += sCamOffset.y;
            camPos.z += sCamOffset.z;
        }
        camPos.x *= TheGfxManager.mScrollSpeed;
        camPos.y *= TheGfxManager.mScrollSpeed;
        camPos.z *= TheGfxManager.mScrollSpeed;
        if (!mVictory) {
            camPos.x += center.x;
            camPos.y += center.y;
            camPos.z += center.z;
        }
    }

    if (pAnim != nullptr) {
        Transform animXfm;
        pAnim->MakeTransform(pAnim->FilterFrame(flElapsed), animXfm, 1);
        sceVu0MulAffineMatrixXyz(&pShipXfm[0][0], &pShipXfm[0][0], &animXfm.mBasisX.x);
    }
    for (int i = 0; i < kBasisRows; ++i) {
        pShipXfm[kTranslationRow][i] *= TheGfxManager.mScrollSpeed;
    }
    if (!mVictory) {
        pShipXfm[kTranslationRow][0] += center.x;
        pShipXfm[kTranslationRow][1] += center.y;
        pShipXfm[kTranslationRow][2] += center.z;
    }
    mShipPos = Vector3{pShipXfm[kTranslationRow][0],
                       pShipXfm[kTranslationRow][1],
                       pShipXfm[kTranslationRow][2],
                       pShipXfm[kTranslationRow][kVec3PaddingFloat]};

    Transform path;
    mTunnel->mGeom->PathXfm(&path, TheGameDb->mSongTick);
    sceVu0MulAffineMatrixXyz(&pShipXfm[0][0], &pShipXfm[0][0], &path.mBasisX.x);
    if (bCam) {
        sceVu0MulAffineMatrixXyz(&pCamXfm->mBasisX.x, &pCamXfm->mBasisX.x, &path.mBasisX.x);
    }
    if (mType == Freestylus::kTypeScratch) {
        mPathAxis = path.mBasisX;
    }
}

void TnlFreestyle::BeginPass() {
    if (mType == Freestylus::kTypeScratch) {
        mTrail->SetShowing(true);
    }
    int bCam;
    int bShip;
    int bShowShip;
    GetPassFlags(&bCam, &bShip, &bShowShip);
    mShip->SetHidden(bShowShip ^ 1);
    mActivator->SetSuppressed(bShowShip ^ 1);
    if (bShip) {
        sFreestylus->Flash();
        sFreestylus->ClearParticles();
    }
}

void TnlFreestyle::EndPass() {
    int bCam;
    int bShip;
    int bShowShip;
    GetPassFlags(&bCam, &bShip, &bShowShip);
    mShip->SetHidden(false);
    mActivator->SetSuppressed(0);
    if (bShip) {
        sFreestylus->Flash();
        mShip->SetOverride(nullptr, 0.0f);
    }
    if (bCam) {
        mTunnel->mCamFX->SetSlide(nullptr, 0.0f);
    }
    mTrail->SetShowing(false);
}

void TnlFreestyle::OnAssembled() {
}

void TnlFreestyle::OnDisassemble() {
}

int TnlFreestyle::GetTrack() {
    const TnlGeom *pGeom = mTunnel->mGeom;
    for (int nTrack = pGeom->mNumTracks - 1; nTrack >= 0; --nTrack) {
        if (pGeom->mTrackGeometry[nTrack].mLeftX <= mShipPos.x) {
            return nTrack;
        }
    }
    return 0;
}

void TnlFreestyle::GetPassFlags(int *pbCam, int *pbShip, int *pbShowShip) {
    if (mVictory) {
        *pbCam = 1;
        *pbShip = (TheGameDb->mCommunity == GameDb::kCommunitySolo);
        *pbShowShip = 1;
        return;
    }
    switch (TheGameDb->mCommunity) {
    case GameDb::kCommunityLocal:
        *pbCam = 0;
        *pbShip = 1;
        break;
    case GameDb::kCommunityOnline:
        *pbCam = TheGameDb->IsLocalPlayer(mPlayer);
        *pbShip = 1;
        break;
    default:
        *pbShip = 1;
        *pbCam = 1;
        break;
    }
    *pbShowShip = 0;
}

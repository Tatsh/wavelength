#include "gfx/playercamfx.h"

#include <cmath>

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "gfx/gfxutil.h"
#include "gfx/tnlgeom.h"
#include "math/rand.h"
#include "math/sine.h"
#include "math/transformops.h"
#include "math/vector2.h"
#include "os/debug.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

constexpr float kPi = 3.14159274f;
constexpr float kHalfPi = 1.57079637f;
constexpr float kDegreesPerHalfTurn = 180.0f;
constexpr float kCenterLateral = 0.5f;
constexpr char kNoPlayer = -1;
constexpr int kCameraIndex = 1;

// The pull back curve and the track curve of the multi algorithm ease with this severity.
constexpr float kCurveSeverity = 10.0f;

// A jiggle smaller than this is dropped.
constexpr float kMinJiggle = 1e-5f;
// A kick picks new jiggle axes only when the spring has nearly stopped.
constexpr float kRestingJiggleSpeed = 0.001f;
constexpr float kRandomCenter = 0.5f;

// NTSC-U/C: 0x003afbc0
float sPullBackTime = 500.0f;
// NTSC-U/C: 0x003afbc4
float sPullBackSeverity = 5.0f;
// The near and far pull backs.
// NTSC-U/C: 0x003afbc8
Vector2 sPullBackLevels = {0.0f, 1.0f};
// NTSC-U/C: 0x003afbd0
float sTutorialPullBackLevel = 2.0f;
// The intro plays over these ticks before the song starts.
// NTSC-U/C: 0x003afbd4
float sIntroStartTick = -7680.0f;
// NTSC-U/C: 0x003afbd8
float sIntroEndTick = -3840.0f;
// NTSC-U/C: 0x003afbdc
float sIntroTilt = 90.0f;
// NTSC-U/C: 0x003afbe0
float sBossIntroTilt = 90.0f;
// Loaded, but read nowhere.
// NTSC-U/C: 0x003afbe4
float sChangeTicks = 480.0f;
// Loaded, but read nowhere.
// NTSC-U/C: 0x003afbe8
float sChangeSeverity = 5.0f;
// NTSC-U/C: 0x003afbec
float sJiggleSpring = 0.5f;
// NTSC-U/C: 0x003afbf0
float sJiggleDamper = 0.5f;
// NTSC-U/C: 0x003afbf4
float sJiggleScale = 1.0f;
// NTSC-U/C: 0x003afbf8
float sScreenJiggleScale = 1.0f;
// NTSC-U/C: 0x003afbfc
const char *sAlgorithmName = "trailing";
// NTSC-U/C: 0x003afc00
float sViewDistance = 2.0f;
// NTSC-U/C: 0x003afc04
float sTrailingFrames = 1000.0f;
// NTSC-U/C: 0x003afc08
int sSkewCorrection = 1;
// NTSC-U/C: 0x0043b670
Vector3 sPullBack{0.0f, 0.0f, 0.0f};
// Maps a tick of the intro to how far its tilt remains.
// NTSC-U/C: 0x0043b680
LinearInterpolator sIntroCurve(1.0f, 0.0f, sIntroStartTick, sIntroEndTick);
// NTSC-U/C: 0x0043b6a0
Vector3 sJiggleVariation{0.0f, 0.0f, 0.0f};
// NTSC-U/C: 0x0043b6b0
std::map<String, PlayerCamFX::Algorithm> sAlgorithms;
// The tilt of the view below the track, -35 degrees.
// NTSC-U/C: 0x0043b6c0
float sViewDownAngle = -0.61086529f;

// Turn a basis about its X axis into dst, which may be src.
inline void RotateBasisAboutX(const Transform &src, float flAngle, Transform &dst) {
    const float flCos = SinApprox(flAngle + kHalfPi);
    const float flSin = SinApprox(flAngle);
    const Vector3 rows[] = {
        Vector3{1.0f, 0.0f, 0.0f}, Vector3{0.0f, flCos, flSin}, Vector3{0.0f, -flSin, flCos}};
    Vector3 basis[3];
    for (int i = 0; i < 3; ++i) {
        basis[i].x =
            (rows[i].x * src.mBasisX.x) + (rows[i].y * src.mBasisY.x) + (rows[i].z * src.mBasisZ.x);
        basis[i].y =
            (rows[i].x * src.mBasisX.y) + (rows[i].y * src.mBasisY.y) + (rows[i].z * src.mBasisZ.y);
        basis[i].z =
            (rows[i].x * src.mBasisX.z) + (rows[i].y * src.mBasisY.z) + (rows[i].z * src.mBasisZ.z);
    }
    dst.mBasisX = basis[0];
    dst.mBasisY = basis[1];
    dst.mBasisZ = basis[2];
}

// Move the camera view_distance back along its Y axis from the frame of the track.
inline void PullBackFromTrack(const Transform &track, Transform &camera) {
    camera.mTranslation.x = (camera.mBasisY.x * -sViewDistance) + track.mTranslation.x;
    camera.mTranslation.z = (camera.mBasisY.z * -sViewDistance) + track.mTranslation.z;
    camera.mTranslation.y = (camera.mBasisY.y * -sViewDistance) + track.mTranslation.y;
}

// Blend a position towards another, leaving it unchanged for a blend of 0.
inline void BlendPosition(Vector3 &pos, const Vector3 &to, float flBlend) {
    if (flBlend == 0.0f) {
        return;
    }
    if (flBlend == 1.0f) {
        pos = to;
        return;
    }
    const float flKeep = 1.0f - flBlend;
    pos.x = (to.x * flBlend) + (pos.x * flKeep);
    pos.y = (to.y * flBlend) + (pos.y * flKeep);
    pos.z = (to.z * flBlend) + (pos.z * flKeep);
}

// Rotate a vector by a basis.
inline Vector3 RotateByBasis(const Transform &xfm, const Vector3 &vec) {
    return Vector3{(vec.x * xfm.mBasisX.x) + (vec.y * xfm.mBasisY.x) + (vec.z * xfm.mBasisZ.x),
                   (vec.x * xfm.mBasisX.y) + (vec.y * xfm.mBasisY.y) + (vec.z * xfm.mBasisZ.y),
                   (vec.x * xfm.mBasisX.z) + (vec.y * xfm.mBasisY.z) + (vec.z * xfm.mBasisZ.z)};
}

// Set the translation of the local transform of a transformable and mark it dirty.
inline void SetLocalTranslation(Rnd::Transformable *pTrans, const Vector3 &pos) {
    pTrans->mDirty = 1;
    float *pRow = pTrans->mLocalXfm[Rnd::kXfmRowCount - 1];
    pRow[0] = pos.x;
    pRow[1] = pos.y;
    pRow[2] = pos.z;
    pRow[kVec3PaddingFloat] = pos.w;
}

inline void Register(const char *pszName, PlayerCamFX::Algorithm algorithm) {
    sAlgorithms[String(pszName)] = algorithm;
}

} // namespace

PlayerCamFX::PlayerCamFX(int nPlayer, int nNumTracks, bool bBoss)
    : mPlayer(nPlayer), mFx(nullptr), mSlide(nullptr), mCam(nullptr),
      mPullBackCurve(1.0f, 0.0f, 0.0f, 1.0f, kCurveSeverity), mPullBackLevel(sPullBackLevels.x),
      mPullingBack(0), mZoom(0.0f), mZoomedOut(0),
      mTrackCurve(1.0f, 1.0f, 0.0f, 1.0f, kCurveSeverity), mFocusPlayer(kNoPlayer),
      mJiggleAxis{0.0f, 0.0f, 1.0f}, mScreenJiggleAxis{0.0f, 0.0f, 1.0f},
      mScreenJiggle{0.0f, 0.0f, 0.0f}, mSliding(0), mSlideBlend(0.0f), mAlgorithm(),
      mBlendAlgorithm(nullptr), mBlend(0.0f) {
    mFx = dynamic_cast<Rnd::Transformable *>(
        Rnd::TheManager.Find(FormatString("tnl cam fx%d", kCameraIndex)));
    mCam = dynamic_cast<Rnd::Cam *>(Rnd::TheManager.Find(FormatString("tnl cam%d", kCameraIndex)));
    mSlide = dynamic_cast<Rnd::Transformable *>(
        Rnd::TheManager.Find(FormatString("tnl cam slide%d", kCameraIndex)));
    if (bBoss) {
        mIntro = dynamic_cast<Rnd::TransAnim *>(Rnd::TheManager.Find("tnl cam boss fly-in_s.tnm"));
        mIntroTilt = sBossIntroTilt;
    } else {
        mIntro = dynamic_cast<Rnd::TransAnim *>(Rnd::TheManager.Find("tnl cam fly-in_s.tnm"));
        mIntroTilt = sIntroTilt;
    }
    const float flCenter = static_cast<float>(nNumTracks - 1) * kCenterLateral;
    mTrack = flCenter;
    mCenterTrack = flCenter;
    mFromTrack = flCenter;
    Reset(true);
    mSkipIntro = 0;
}

PlayerCamFX::~PlayerCamFX() {
}

void PlayerCamFX::Poll(GfxTunnel *pTunnel, float flTick, float flTime) {
    const bool bLocal = (TheGameDb->mCommunity == GameDb::kCommunityLocal);
    if (mFx == nullptr) {
        return;
    }

    Transform track;
    (this->*(mAlgorithm->second))(pTunnel, track, mCamera, mPlayer, bLocal);
    if (mBlendAlgorithm != nullptr) {
        Transform blendTrack;
        Transform blendCamera;
        (this->*mBlendAlgorithm)(pTunnel, blendTrack, blendCamera, mPlayer, bLocal);
        BlendPosition(mCamera.mTranslation, blendCamera.mTranslation, mBlend);
        InterpBasis(&mCamera.mBasisX.x, &blendCamera.mBasisX.x, &mCamera.mBasisX.x, mBlend);
        if (!bLocal) {
            BlendPosition(track.mTranslation, blendTrack.mTranslation, mBlend);
            InterpBasis(&track.mBasisX.x, &blendTrack.mBasisX.x, &track.mBasisX.x, mBlend);
        }
    }
    // The original leaves the fourth word of the cleared translation as stack garbage.
    SetLocalTranslation(mFx, Vector3{0.0f, 0.0f, 0.0f});

    if (!bLocal) {
        if (mPullingBack) {
            if (mPullBackCurve.mX1 <= flTime) {
                mPullingBack = 0;
                mPullBackLevel = mPullBackCurve.mY1;
            } else {
                mPullBackLevel = mPullBackCurve.ATanInterpolator::Interp(flTime);
            }
        }
        if (flTick < sIntroEndTick && TheGameDb->mRuleSet != GameDb::kRuleSetRemix &&
            !TheGameDb->mTutorial && !mSkipIntro) {
            PlayIntro(flTick);
        } else {
            mView.mBasisX = mCamera.mBasisX;
            mView.mBasisY = mCamera.mBasisY;
            mView.mBasisZ = mCamera.mBasisZ;
        }
        const Vector3 pullBack{sPullBack.x * mPullBackLevel,
                               sPullBack.y * mPullBackLevel,
                               sPullBack.z * mPullBackLevel};
        const Vector3 offset = RotateByBasis(track, pullBack);
        mView.mTranslation.x = offset.x + mCamera.mTranslation.x;
        mView.mTranslation.y = offset.y + mCamera.mTranslation.y;
        mView.mTranslation.z = offset.z + mCamera.mTranslation.z;
    } else {
        mView = mCamera;
    }

    if (mSliding) {
        BlendPosition(mView.mTranslation, mSlideXfm.mTranslation, mSlideBlend);
        InterpBasis(&mView.mBasisX.x, &mSlideXfm.mBasisX.x, &mView.mBasisX.x, mSlideBlend);
    }
    mSlide->SetLocalXfm(mView);

    float flJiggle = mJiggle.Apply(0.0f);
    if (flJiggle < kMinJiggle) {
        flJiggle = 0.0f;
    }
    const float flJiggleScale = flJiggle * sJiggleScale;
    Vector3 jiggle{mJiggleAxis.x * flJiggleScale,
                   mJiggleAxis.y * flJiggleScale,
                   mJiggleAxis.z * flJiggleScale};
    const float flScreenJiggleScale = flJiggle * sScreenJiggleScale;
    mScreenJiggle.x = mScreenJiggleAxis.x * flScreenJiggleScale;
    mScreenJiggle.z = mScreenJiggleAxis.z * flScreenJiggleScale;
    mScreenJiggle.y = mScreenJiggleAxis.y * flScreenJiggleScale;
    const float *pTranslation = mFx->mLocalXfm[Rnd::kXfmRowCount - 1];
    jiggle.x += pTranslation[0];
    jiggle.y += pTranslation[1];
    jiggle.z += pTranslation[2];
    SetLocalTranslation(mFx, jiggle);
}

void PlayerCamFX::SetSlide(const Transform *pXfm, float flBlend) {
    if (pXfm == nullptr) {
        mSliding = 0;
        return;
    }
    mSlideBlend = flBlend;
    mSliding = 1;
    mSlideXfm = *pXfm;
}

void PlayerCamFX::PlayIntro(float flTick) {
    Transform anim;
    mIntro->EvalFrame(mIntro->FilterFrame(flTick), &anim.mBasisX.x, 1);
    sceVu0MulAffineMatrixXyz(&mCamera.mBasisX.x, &mCamera.mBasisX.x, &anim.mBasisX.x);
    const float flTilt = sIntroCurve.Eval(flTick) * mIntroTilt;
    RotateBasisAboutX(mCamera, flTilt, mView);
}

void PlayerCamFX::LookAlong(
    GfxTunnel *pTunnel, Transform &track, Transform &camera, int, bool, float flTrack) {
    const float flTick = TheGameDb->mSongTick;
    TnlGeom *pGeom = pTunnel->mGeom;
    pGeom->BlendCell(&track, true, false, flTrack, flTick, kCenterLateral);
    pGeom->BlendCell(&camera, true, false, flTrack, flTick - sTrailingFrames, kCenterLateral);
    RotateBasisAboutX(camera, sViewDownAngle, camera);
    if (sSkewCorrection) {
        camera.mBasisX = track.mBasisX;
        CrossVec3(&camera.mBasisX.x, &camera.mBasisY.x, &camera.mBasisZ.x);
        Vec3Normalize(&camera.mBasisZ.x, &camera.mBasisZ.x);
        CrossVec3(&camera.mBasisY.x, &camera.mBasisZ.x, &camera.mBasisX.x);
    }
    PullBackFromTrack(track, camera);
}

void PlayerCamFX::Trailing(
    GfxTunnel *pTunnel, Transform &track, Transform &camera, int nPlayer, bool bLocal) {
    LookAlong(
        pTunnel, track, camera, nPlayer, bLocal, pTunnel->mGeom->GetPlayer(nPlayer)->mPosition);
}

void PlayerCamFX::TrailingFixed(
    GfxTunnel *pTunnel, Transform &track, Transform &camera, int nPlayer, bool bLocal) {
    LookAlong(pTunnel,
              track,
              camera,
              nPlayer,
              bLocal,
              static_cast<float>(pTunnel->mNumTracks - 1) * kCenterLateral);
}

void PlayerCamFX::Solo(GfxTunnel *pTunnel, Transform &track, Transform &camera, int nPlayer, bool) {
    const float flTick = TheGameDb->mSongTick;
    TnlGeom *pGeom = pTunnel->mGeom;
    pGeom->BlendCell(
        &track, true, false, pGeom->GetPlayer(nPlayer)->mPosition, flTick, kCenterLateral);
    RotateBasisAboutX(track, sViewDownAngle, camera);
    PullBackFromTrack(track, camera);
}

void PlayerCamFX::Multi(GfxTunnel *pTunnel, Transform &track, Transform &camera, int, bool) {
    const float flTick = TheGameDb->mSongTick;
    TnlGeom *pGeom = pTunnel->mGeom;
    float flTrack;
    if (mFocusPlayer >= 0) {
        flTrack = pGeom->GetPlayer(mFocusPlayer)->mPosition;
    } else {
        flTrack = mCenterTrack;
    }
    const float flMove = mTrackCurve.Eval(flTick);
    mTrack = ((flTrack - mFromTrack) * flMove) + mFromTrack;
    pGeom->BlendTrack(&track, true, mTrack, flTick, kCenterLateral);
    RotateBasisAboutX(track, sViewDownAngle, camera);
    PullBackFromTrack(track, camera);
}

void PlayerCamFX::LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *, int nOption) {
    FindConfigFloat(pConfig, pDefaults, "camera_intro_tilt", &sIntroTilt, true);
    FindConfigFloat(pConfig, pDefaults, "camera_boss_intro_tilt", &sBossIntroTilt, true);
    sIntroTilt = (sIntroTilt * kPi) / kDegreesPerHalfTurn;
    sBossIntroTilt = (sBossIntroTilt * kPi) / kDegreesPerHalfTurn;
    FindConfigVector3(pConfig, pDefaults, "view_pull_back", &sPullBack, true);
    FindConfigFloat(pConfig, pDefaults, "view_pull_back_time", &sPullBackTime, true);
    FindConfigFloat(pConfig, pDefaults, "view_pull_back_severity", &sPullBackSeverity, true);
    sPullBack.x *= TheGfxManager.mScrollSpeed;
    sPullBack.y *= TheGfxManager.mScrollSpeed;
    sPullBack.z *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(
        pConfig, pDefaults, "view_tutorial_pull_back_level", &sTutorialPullBackLevel, true);
    Vector2 levels;
    FindConfigVector2(pConfig, pDefaults, "view_pull_back_base_levels", &levels, true);
    sPullBackLevels.x = levels.x;
    sPullBackLevels.y = levels.y;

    if (!nOption) {
        Register("trailing", &PlayerCamFX::Trailing);
        Register("trailing_fixed", &PlayerCamFX::TrailingFixed);
        Register("solo", &PlayerCamFX::Solo);
        Register("multi", &PlayerCamFX::Multi);
    }

    FindConfigFloat(pConfig, pDefaults, "view_distance", &sViewDistance, true);
    if (FindConfigFloat(pConfig, pDefaults, "view_down_angle", &sViewDownAngle, true)) {
        sViewDownAngle = (sViewDownAngle * kPi) / kDegreesPerHalfTurn;
    }
    FindConfigFloat(pConfig, pDefaults, "view_trailing_frames", &sTrailingFrames, true);
    int nSkewCorrection;
    if (FindConfigInt(pConfig, pDefaults, "view_skew_correction", &nSkewCorrection, true)) {
        sSkewCorrection = (nSkewCorrection != 0);
    }
    sViewDistance *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(
        pConfig, pDefaults, "camera_multi_player_change_severity", &sChangeSeverity, true);
    FindConfigFloat(pConfig, pDefaults, "camera_multi_player_change_ticks", &sChangeTicks, true);
    FindConfigFloat(pConfig, pDefaults, "camera_jiggle_spring", &sJiggleSpring, true);
    FindConfigFloat(pConfig, pDefaults, "camera_jiggle_damper", &sJiggleDamper, true);
    FindConfigFloat(pConfig, pDefaults, "camera_jiggle_scale", &sJiggleScale, true);
    sJiggleScale *= TheGfxManager.mScrollSpeed;
    FindConfigVector3(pConfig, pDefaults, "camera_jiggle_variation", &sJiggleVariation, true);
    FindConfigFloat(pConfig, pDefaults, "camera_jiggle_2d_mult", &sScreenJiggleScale, true);
    FindConfigSymbol(pConfig, pDefaults, "camera_alg", &sAlgorithmName, false);
}

void PlayerCamFX::SetZoomedOut(int nZoomedOut) {
    mZoomedOut = nZoomedOut;
    UpdateZoom(false);
}

void PlayerCamFX::SetZoom(float flZoom) {
    mZoom = flZoom;
    UpdateZoom(false);
}

void PlayerCamFX::UpdateZoom(bool bImmediate) {
    float flLevel;
    if (mZoom != 0.0f) {
        flLevel = mZoom * sPullBackLevels.y;
    } else if (mZoomedOut) {
        flLevel = sPullBackLevels.y;
    } else {
        flLevel = sPullBackLevels.x;
    }
    if (bImmediate) {
        mPullBackLevel = flLevel;
        mPullingBack = 0;
        return;
    }
    if (!mPullingBack && flLevel == mPullBackLevel) {
        return;
    }
    const float flNow = TheGameDb->mSongTime;
    mPullingBack = 1;
    mPullBackCurve.Reset(mPullBackLevel, flLevel, flNow, flNow + sPullBackTime, sPullBackSeverity);
}

void PlayerCamFX::Reset(bool bNearPullBack) {
    Transform identity;
    identity.mBasisX = Vector3{1.0f, 0.0f, 0.0f};
    identity.mBasisY = Vector3{0.0f, 1.0f, 0.0f};
    identity.mBasisZ = Vector3{0.0f, 0.0f, 1.0f};
    identity.mTranslation = Vector3{0.0f, 0.0f, 0.0f};
    mFx->SetLocalXfm(identity);
    mFx->UpdateWorldXfm(nullptr, 0);

    if (TheGameDb->mTutorial) {
        mPullingBack = 0;
        mPullBackLevel = sTutorialPullBackLevel;
    } else if (bNearPullBack) {
        mPullingBack = 0;
        mPullBackLevel = sPullBackLevels.x;
    } else {
        UpdateZoom(true);
    }

    mAlgorithm = sAlgorithms.find(String(sAlgorithmName));
    mTrackCurve.Reset(1.0f, 1.0f, 0.0f, 1.0f);
    mFocusPlayer = kNoPlayer;
    mTrack = mCenterTrack;
    mFromTrack = mCenterTrack;
    mJiggleAxis = Vector3{0.0f, 0.0f, 1.0f};
    mScreenJiggleAxis = Vector3{0.0f, 0.0f, 1.0f};
    mScreenJiggle = Vector3{0.0f, 0.0f, 0.0f};
    mJiggle.mLevel = 0.0f;
    mJiggle.mVel = 0.0f;
    mSkipIntro = 1;
    mJiggle.mSpring = sJiggleSpring;
    mJiggle.mDamper = sJiggleDamper;
}

void PlayerCamFX::Kick(float flAmount) {
    if (std::fabs(mJiggle.mVel) < kRestingJiggleSpeed) {
        const float flX = sJiggleVariation.x * (RandomFraction() - kRandomCenter);
        const float flY = sJiggleVariation.y * (RandomFraction() - kRandomCenter);
        mJiggleAxis.x = flX;
        mJiggleAxis.y = flY;
        mJiggleAxis.z = (sJiggleVariation.z * (RandomFraction() - kRandomCenter)) + 1.0f;
        Vec3Normalize(&mJiggleAxis.x, &mJiggleAxis.x);

        const float flScreenX = sJiggleVariation.x * (RandomFraction() - kRandomCenter);
        mScreenJiggleAxis.x = flScreenX;
        mScreenJiggleAxis.y = 0.0f;
        mScreenJiggleAxis.z = (sJiggleVariation.z * (RandomFraction() - kRandomCenter)) + 1.0f;
        Vec3Normalize(&mScreenJiggleAxis.x, &mScreenJiggleAxis.x);
    }
    mJiggle.mVel += flAmount;
}

void PlayerCamFX::SetBlendAlgorithm(const char *pszName, float flBlend) {
    mBlend = flBlend;
    if (pszName == nullptr) {
        mBlendAlgorithm = nullptr;
        return;
    }
    const auto it = sAlgorithms.find(String(pszName));
    if (it == sAlgorithms.end()) {
        DebugNotify("could not find algorithm %s", pszName);
        mBlendAlgorithm = nullptr;
    } else {
        mBlendAlgorithm = it->second;
    }
}

void PlayerCamFX::SetBlend(float flBlend) {
    mBlend = flBlend;
}

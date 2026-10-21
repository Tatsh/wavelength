#include "gfx/tnlplayer.h"

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "math/sine.h"
#include "math/transform.h"
#include "os/string.h"
#include "rnd/rndmanager.h"
#include "rnd/rndtransformable.h"

namespace {

// A multiplier below this draws the track in its own colour.
constexpr int kStreakMultiplier = 2;

// `player_cam_base_pitch` is in degrees, and a half turn is this many radians and degrees.
constexpr float kPi = 3.1415927f;
constexpr float kHalfTurnDegrees = 180.0f;

// A quarter turn, which turns a sine into a cosine.
constexpr float kQuarterTurn = 1.5707964f;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

} // namespace

TnlPlayer::TnlPlayer(int nPlayer,
                     int nCam,
                     const char *pszColor,
                     GfxTunnel *pTunnel,
                     TnlGeom *pGeom,
                     int nFreestyleType,
                     char nExcludedType,
                     char nOtherExcludedType)
    : mCamNumber(nCam + 1), mCam(nullptr), mIntro(nullptr), mLocalView(nullptr), mTunnel(pTunnel),
      mGeom(pGeom), mReserved18(0), mPlayer(nPlayer), mCamIndex(nCam), mColor(&mTrackColor),
      mActivator(pTunnel, nPlayer, nCam + 1, pszColor), mGrid(pTunnel, nCam + 1),
      mCursor(pTunnel, nPlayer, pszColor, nExcludedType, nOtherExcludedType),
      mFreestyle(nPlayer, nFreestyleType, TheGfxManager.GetShip(nPlayer), &mActivator, pTunnel) {
    mTrackColor = Color{1.0f, 1.0f, 1.0f, 1.0f};
    mStreakColor = Color{1.0f, 1.0f, 1.0f, 1.0f};
    TnlGeom::Player *pPlayer = mGeom->GetPlayer(nPlayer);
    if (nCam < 0) {
        pPlayer->SetCamSlide(nullptr);
        mCam = nullptr;
        mIntro = nullptr;
        return;
    }
    mCam = FindObject<RndCam>(FormatString("tnl cam%d", mCamNumber));
    mLocalView = FindObject<RndView>(FormatString("tnl local%d.view", mCamNumber));
    pPlayer->SetCamSlide(FindObject<RndTransformable>(FormatString("tnl cam slide%d", mCamNumber)));
    mIntro = FindObject<RndTransAnim>(FormatString("tnl cam intro%d.tnm", TheGameDb->GetNumPads()));
}

TnlPlayer::~TnlPlayer() = default;

void TnlPlayer::LoadCamXfm(DataArray *pConfig, DataArray *pDefaults, RndCam *pCam) {
    float flPitch;
    FindConfigFloat(pConfig, pDefaults, "player_cam_base_pitch", &flPitch, true);
    Vector3 offset;
    FindConfigVector3(pConfig, pDefaults, "player_cam_base_offset", &offset, true);
    flPitch = (flPitch * kPi) / kHalfTurnDegrees;
    offset.x *= TheGfxManager.GetScrollSpeed();
    offset.y *= TheGfxManager.GetScrollSpeed();
    offset.z *= TheGfxManager.GetScrollSpeed();
    const float flCos = SinApprox(flPitch + kQuarterTurn);
    const float flSin = SinApprox(flPitch);
    const Transform xfm{Vector3{1.0f, 0.0f, 0.0f},
                        Vector3{0.0f, flCos, flSin},
                        Vector3{0.0f, -flSin, flCos},
                        offset};
    pCam->SetLocalXfm(xfm);
    pCam->SetDirty(1);
}

void TnlPlayer::LoadConfig(
    DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption, int bFreestyle) {
    const char cColor = TheGameDb->GetPlayerColor(mPlayer)[0];
    mActivator.LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    mCursor.LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    mFreestyle.LoadConfig(pConfig, pDefaults, pTunnel, nOption, bFreestyle);
    if (mIntro != nullptr) {
        mIntro->SetTrans(nullptr);
    }
    if (mCam != nullptr) {
        LoadCamXfm(pConfig, pDefaults, mCam);
    }
    mTrackColor = Color{1.0f, 1.0f, 1.0f, 1.0f};
    mStreakColor = Color{1.0f, 1.0f, 1.0f, 1.0f};
    const bool bFail = TheGameDb->GetCommunity() != GameDb::kCommunitySolo;
    FindConfigColor(
        pConfig, pDefaults, FormatString("track_color_%c", cColor), &mTrackColor, bFail);
    FindConfigColor(
        pConfig, pDefaults, FormatString("track_color_%c_streak", cColor), &mStreakColor, bFail);
}

bool TnlPlayer::SelectColor(int nMultiplier) {
    Color *pColor = (nMultiplier < kStreakMultiplier) ? &mTrackColor : &mStreakColor;
    if (pColor == mColor) {
        return false;
    }
    mColor = pColor;
    return true;
}

void TnlPlayer::Update([[maybe_unused]] GfxArena *pArena, float flTicks, float flTime) {
    mActivator.Update(flTime);
    mGrid.Update();
    const int nTrack = mGeom->GetPlayer(mPlayer)->GetTrack();
    mCursor.Poll(mTunnel->IsTrackCaptured(nTrack), flTicks, flTime);
}

void TnlPlayer::UpdateTrail([[maybe_unused]] float flTicks, [[maybe_unused]] float flTime) {
    mFreestyle.UpdateTrail();
}

#include "gfx/tnlcatcher.h"

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "gfx/gfxutil.h"
#include "gfx/tnlactivator.h"
#include "math/transform.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

// The letter of the views of a solo game, whatever the colour of the player.
constexpr char kSoloLetter = 's';

// The pose of the animation of a catcher at rest, raised, and lowered past its rest.
constexpr float kPoseRest = 1000.0f;
constexpr float kPoseRaised = 2000.0f;
constexpr float kPoseLowered = 2400.0f;

// The speed the pose of a new catcher moves at.
constexpr float kPoseSpeed = 5.0f;

// The targets sit across the track from this place, this far apart.
constexpr float kFirstTargetX = 0.205f;
constexpr float kTargetSpacing = 0.295f;
constexpr float kTrackCenter = 0.5f;

// A hit animation plays for this long.
constexpr float kHitLength = 160.0f;

// Before the song the catcher rises over the ticks from kRiseStart, kRiseLength long, through
// kRiseFrames frames of its pose that end at frame 0.
constexpr float kRiseStart = -3840.0f;
constexpr float kRiseLength = 480.0f;
constexpr float kRiseFrames = 100.0f;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

} // namespace

float TnlCatcher::sTargetSize = 0.2f;
float TnlCatcher::sTargetOffset = 0.01f;
int TnlCatcher::sTargetsOnTop = 0;
float TnlCatcher::sBumpShowTime = 150.0f;
float TnlCatcher::sBumpEndTime = 270.0f;

TnlCatcher::TnlCatcher(const char *pszColor, GfxTunnel *pTunnel)
    : mView(nullptr), mShown(1), mEnabled(1), mAnim(kPoseRest, nullptr, nullptr, kPoseSpeed),
      mMultiplied(0), mBumping(0) {
    const char cLetter =
        (TheGameDb->GetCommunity() == GameDb::kCommunitySolo) ? kSoloLetter : pszColor[0];
    mView = FindObject<RndView>(FormatString("catcher_%c", pszColor[0]));
    mView->ClearDraws();
    const float flPanelWidth = pTunnel->PanelWidth();
    for (int i = 0; i < kNumTargets; ++i) {
        mTargets[i] = FindObject<RndView>(FormatString("target %d_%c.view", i, cLetter));
        mView->AddDraw(mTargets[i], nullptr);
        mView->AddTrans(mTargets[i]);
        ResetXfmTree(mTargets[i], false);
        const float flX = ((static_cast<float>(i) * kTargetSpacing) + kFirstTargetX) - kTrackCenter;
        const Transform xfm{Vector3{sTargetSize, 0.0f, 0.0f},
                            Vector3{0.0f, sTargetSize, 0.0f},
                            Vector3{0.0f, 0.0f, 1.0f},
                            Vector3{flX * flPanelWidth, 0.0f, sTargetOffset}};
        mTargets[i]->mLocalXfm = xfm;
        mTargets[i]->mDirty = 1;
        mTargets[i]->UpdateWorldXfm(nullptr, 0);
        mTargetHits[i] =
            FindObject<RndAnimatable>(FormatString("target hit %d_%c.tnm", i, cLetter));
        mHitTimes[i] = kNoHit;
    }

    mBumpFlare = FindObject<RndMatAnim>(FormatString("target bump flare_%c.mnm", cLetter));
    if (mBumpFlare != nullptr) {
        sBumpEndTime = mBumpFlare->mKeysOwner->mAlphaKeys.back().frame;
    }
    mBumpFlareMesh = FindObject<RndDrawable>(FormatString("target bump flare 0_%c.mesh", cLetter));
    if (mBumpFlare != nullptr) {
        mColor = mBumpFlare->mKeysOwner->mBaseKeys.back().value;
    }
    mAmbient = FindObject<RndAnimatable>(FormatString("target ambient_%c.view", cLetter));
    mAnim.SetAnim(FindObject<RndAnimatable>(FormatString("target anim_%c.view", cLetter)));
    if (sTargetsOnTop) {
        for (auto *pTarget : mTargets) {
            SetZModeTree(pTarget, RndMesh::kZModeDisable, RndMesh::kZFuncLess);
        }
    }
    SetMultiplied(0);
    ResetTargets();
}

TnlCatcher::~TnlCatcher() = default;

void TnlCatcher::SetAlpha(float) {
}

void TnlCatcher::SetMultiplied(int bMultiplied) {
    mMultiplied = bMultiplied;
}

void TnlCatcher::AttachTo(RndView *pParent) {
    pParent->AddTrans(mView);
    pParent->AddDraw(mView, nullptr);
    if (mBumpFlareMesh != nullptr) {
        pParent->AddDraw(mBumpFlareMesh, nullptr);
    }
}

void TnlCatcher::Hit(int nTarget) {
    if (mView->mShowing) {
        mHitTimes[nTarget] = TheGameDb->GetSongTime();
    }
}

void TnlCatcher::SetEnabled(int bEnabled) {
    mEnabled = bEnabled;
}

void TnlCatcher::SetShown(int bShown) {
    mShown = bShown;
}

void TnlCatcher::ResetTargets() {
    for (int i = kNumTargets - 1; i >= 0; --i) {
        mHitTimes[i] = kNoHit;
    }
}

void TnlCatcher::StopBump(TnlGeom *pGeom, int nPlayer) {
    mBumping = 0;
    if (mBumpFlareMesh != nullptr) {
        mBumpFlareMesh->SetShowing(0);
    }
    pGeom->GetPlayer(nPlayer)->SetHoldCam(0);
}

void TnlCatcher::StartBump(const TnlActivator *pAttacker, TnlGeom *pGeom, int nPlayer) {
    if (pAttacker == nullptr) {
        return;
    }
    mAnim.Jump(kPoseRest, kPoseRest);
    mBumpFlare->mKeysOwner->mBaseKeys.back().value = pAttacker->mCatcher.mColor;
    mBumpFlareMesh->SetShowing(1);
    pGeom->GetPlayer(nPlayer)->SetHoldCam(1);
    mBumping = 1;
}

void TnlCatcher::SetRaised(bool bRaised) {
    const float flPose = mAnim.GetValue();
    if (bRaised) {
        if (kPoseRaised <= flPose) {
            return;
        }
        if (flPose <= kPoseRest) {
            mAnim.Jump(kPoseRest, kPoseRaised);
        } else {
            mAnim.SetTarget(kPoseRaised);
        }
        return;
    }
    if (flPose <= kPoseRest) {
        return;
    }
    if (kPoseRaised <= flPose) {
        mAnim.Jump(kPoseLowered, kPoseRest);
    } else {
        mAnim.SetTarget(kPoseRest);
    }
}

void TnlCatcher::Update(
    TnlGeom *pGeom, int nPlayer, bool bBumped, float flDelta, float flBumpTime) {
    const float flTick = TheGameDb->GetSongTick();
    const float flTime = TheGameDb->GetSongTime();
    for (int i = 0; i < kNumTargets; ++i) {
        float flFrame = 0.0f;
        if (mHitTimes[i] != kNoHit) {
            flFrame = flTime - mHitTimes[i];
            if (kHitLength < flFrame) {
                mHitTimes[i] = kNoHit;
                flFrame = 0.0f;
            }
        }
        mTargetHits[i]->SetFrame(flFrame);
    }

    bool bShow = mEnabled && mShown;
    if (flTick <= 0.0f) {
        float flRise = (flTick - kRiseStart) / kRiseLength;
        if (1.0f < flRise) {
            flRise = 1.0f;
        } else if (flRise < 0.0f) {
            flRise = 0.0f;
        }
        mAnim.Jump(kPoseRest, kPoseRest);
        mAnim.GetAnim()->SetFrame((flRise * kRiseFrames) - kRiseFrames);
    } else {
        mAnim.Update(flDelta, false);
    }

    if (mBumping) {
        const float flSinceBump = flTime - flBumpTime;
        bool bFlare = true;
        if (sBumpShowTime <= flSinceBump) {
            bShow = false;
            if (sBumpEndTime <= flSinceBump) {
                mBumping = 0;
                mBumpFlareMesh->SetShowing(0);
                pGeom->GetPlayer(nPlayer)->SetHoldCam(0);
                bFlare = false;
            }
        }
        if (bFlare) {
            mBumpFlare->SetFrame(flSinceBump);
        }
    } else if (bBumped) {
        bShow = false;
    }
    mAmbient->SetFrame(flTime);
    mView->SetShowing(bShow);
}

void TnlCatcher::LoadConfig(DataArray *pConfig,
                            DataArray *pDefaults,
                            [[maybe_unused]] GfxTunnel *pTunnel,
                            [[maybe_unused]] int nOption) {
    FindConfigBool(pConfig, pDefaults, "targets_always_on_top", &sTargetsOnTop, true);
    FindConfigFloat(pConfig, pDefaults, "target_size", &sTargetSize, true);
    FindConfigFloat(pConfig, pDefaults, "target_offset", &sTargetOffset, true);
    sTargetSize *= TheGfxManager.GetScrollSpeed();
    sTargetOffset *= TheGfxManager.GetScrollSpeed();
}

#include "app/ovyscore.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "math/color.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

// The sentinels of mScoreTime: no score yet, and a score already shown.
constexpr float kNoScoreTime = -1e9f;
constexpr float kScoreShown = 1e9f;

// The time a new score waits before it shows, in milliseconds.
constexpr float kScoreDelay = 600.0f;

// The ease and speed of the bar's growth.
constexpr float kBarGrowPower = 3.0f;
constexpr float kBarGrowSpeed = 0.1f;

// The frames of the bar animation per unit of share.
constexpr float kBarFramesPerShare = 1000.0f;

// The speed of the slide of the points.
constexpr float kPointsSlideSpeed = 1.0f;

// The frame of the slide of the points at which the points are out of sight.
constexpr float kPointsHiddenFrame = 480.0f;

template <typename T>
T *FindIndexed(const char *pszFormat, int nIndex) {
    return dynamic_cast<T *>(TheManager.Find(FormatString(pszFormat, Overlay::sHudPrefix, nIndex)));
}

void SetTranslation(RndTransformable *pTrans, const Vector3 &position) {
    pTrans->mDirty = 1;
    pTrans->mLocalXfm.mTranslation = position;
}

} // namespace

float OvyScore::sMoveInTicks = 480.0f;
float OvyScore::sBarGrowTicks = 480.0f;
float OvyScore::sStableTicks = 960.0f;
float OvyScore::sFadeTicks = 480.0f;
float OvyScore::sMoveOutTicks = 480.0f;
// Yes, the binary leaves these places, fourth words included, at zero until the configuration.
Vector3 OvyScore::sCheckpointPositions[kNumCheckpointPositions] = {
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
};

OvyScore::OvyScore(int nPlayer, int nIndex, RndView *pHudView)
    : HideablePanel(nullptr, nullptr, false), mPowerup(nullptr), mScoreText(nullptr),
      mPlayer(static_cast<signed char>(nPlayer)), mPointsSlide(nullptr), mState(kStateIdle),
      mTop(nullptr), mMesh(nullptr), mMove(0.0f, 0.0f, 0.0f, 1.0f), mBar(nullptr), mBarMat(nullptr),
      mBarGrow(0.0f,
               nullptr,
               new InvExpInterpolator(0.0f, 0.0f, 1.0f, 1.0f, kBarGrowPower),
               kBarGrowSpeed) {
    mTopPos.x = 0.0f;
    mTopPos.z = 0.0f;
    mTopPos.y = 0.0f;
    mCheckpointPos = nullptr;
    mMesh = FindIndexed<RndDrawable>("%s score%d.mesh", nIndex);
    SetObjects(
        FormatString("%s score%d.tnm", Overlay::sHudPrefix, nIndex), mMesh->mName.c_str(), false);
    mScoreText = FindIndexed<RndText>("%s score%d.txt", nIndex);
    auto *pPoints = FindIndexed<RndMesh>("%s pts%d.mesh", nIndex);
    if (TheGameDb->GetCommunity() == GameDb::kCommunitySolo ||
        TheGameDb->GetRuleSet() != GameDb::kRuleSetGame) {
        mMesh = nullptr;
        mTop = nullptr;
    } else {
        mTop = FindIndexed<RndTransformable>("%s score top%d.view", nIndex);
        mBar = FindIndexed<RndMesh>("%s score bar%d.mesh", nIndex);
        mBarMat =
            dynamic_cast<RndMat *>(TheManager.Find(FormatString("HUD score bar%d.mat", nIndex)));
        mBarGrow.SetAnim(FindIndexed<RndAnimatable>("%s score bar%d.anim", nIndex));
        mBar->SetShowing(false);
        mTopPos = mTop->mLocalXfm.mTranslation;
        mBackgroundMat =
            dynamic_cast<RndMat *>(TheManager.Find(FormatString("HUD score%d bg.mat", nIndex)));
        const Color *pColor = TheGfxManager.GetPlayerBgColor(nPlayer);
        mBackgroundMat->SetBaseColor(*pColor);
        mBarMat->SetBaseColor(*pColor);
        static_cast<RndDrawable *>(pHudView)->RemoveDraw(mMesh);
    }
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetGame &&
        TheGameDb->GetCommunity() == GameDb::kCommunityLocal) {
        auto *pPointsAnim = FindIndexed<RndAnimatable>("%s pts%d hide.tnm", nIndex);
        pPoints->SetShowing(true);
        mPointsSlide =
            std::make_unique<RampAnimator>(0.0f, pPointsAnim, nullptr, kPointsSlideSpeed);
    } else if (pPoints != nullptr) {
        pPoints->SetShowing(false);
    }
    if (TheGameDb->GetRuleSet() != GameDb::kRuleSetRemix &&
        TheGameDb->GetRuleSet() != GameDb::kRuleSetDuel && TheGameDb->IsLocalPlayer(nPlayer)) {
        mPowerup = std::make_unique<HudPowerup>(nIndex, pHudView);
    }
    Reset();
}

OvyScore::~OvyScore() = default;

void OvyScore::Show(bool bShow) {
    HideablePanel::Show(bShow && TheGfxManager.GetVictoryLap() == 0);
}

void OvyScore::Reset() {
    mScore = 0;
    mScoreTime = kNoScoreTime;
    mBarGrow.Jump(0.0f, 0.0f);
    if (mBar != nullptr) {
        mBar->SetShowing(false);
    }
    if (mTop != nullptr) {
        SetTranslation(mTop, mTopPos);
    }
    mState = kStateIdle;
    if (mPowerup != nullptr) {
        mPowerup->Reset();
    }
    if (mPointsSlide != nullptr) {
        mPointsSlide->Jump(0.0f, 0.0f);
    }
}

void OvyScore::SetScore(int nScore) {
    mScore = nScore;
    const GameDb *pDb = TheGameDb;
    if (pDb->GetCommunity() == GameDb::kCommunitySolo ||
        (pDb->GetCommunity() == GameDb::kCommunityOnline && TheGameDb->IsLocalPlayer(mPlayer)) ||
        TheGameDb->GetRuleSet() == GameDb::kRuleSetDuel) {
        mScoreTime = TheGameDb->GetSongTime();
    } else {
        mScoreTime = kNoScoreTime;
    }
}

void OvyScore::Poll(float fDelta, float fUnused, float fDeltaTicks) {
    HideablePanel::Poll();
    const float fTick = TheGameDb->GetSongTick();
    switch (mState) {
    case kStateMoveIn:
    case kStateMoveOut: {
        float fProgress;
        if (mMove.GetX1() <= fTick) {
            if (mState == kStateMoveIn) {
                mState = kStateGrow;
                mBar->SetShowing(true);
                fProgress = 1.0f;
            } else {
                fProgress = 0.0f;
                mState = kStateIdle;
                if (mPowerup != nullptr) {
                    mPowerup->mEnabled = 1;
                    mPowerup->UpdateSlide();
                }
                if (mPointsSlide != nullptr) {
                    mPointsSlide->SetTarget(fProgress);
                }
            }
        } else {
            fProgress = mMove.LinearInterpolator::Interp(fTick);
        }
        Vector3 position;
        if (fProgress == 0.0f) {
            position = mTopPos;
        } else if (fProgress == 1.0f) {
            position = *mCheckpointPos;
        } else {
            const float fRest = 1.0f - fProgress;
            position = *mCheckpointPos;
            position.x = mCheckpointPos->x * fProgress + mTopPos.x * fRest;
            position.y = mCheckpointPos->y * fProgress + mTopPos.y * fRest;
            position.z = mCheckpointPos->z * fProgress + mTopPos.z * fRest;
        }
        SetTranslation(mTop, position);
        break;
    }
    case kStateGrow:
        mBarGrow.Update(fDelta, false);
        if (mStableEnd <= fTick) {
            mState = kStateFade;
            mMove.Reset(1.0f, 0.0f, mStableEnd, mStableEnd + sFadeTicks);
        }
        break;
    case kStateFade: {
        Color color = mBarMat->mEdgeColor;
        if (mMove.GetX1() <= fTick) {
            mState = kStateMoveOut;
            mMove.Reset(1.0f, 0.0f, mMove.GetX1(), mMove.GetX1() + sMoveOutTicks);
            color.a = 0.0f;
            mBar->SetShowing(false);
        } else {
            color.a = mMove.LinearInterpolator::Interp(fTick);
        }
        mBarMat->SetEdgeColor(color);
        mBarMat->SetAlpha(color.a);
        break;
    }
    default:
        break;
    }
    if (mScoreTime != kScoreShown && kScoreDelay < TheGameDb->GetSongTime() - mScoreTime) {
        mScoreText->SetText(FormatString("%d", mScore));
        mScoreTime = kScoreShown;
    }
    if (mPowerup != nullptr) {
        mPowerup->Poll(fDelta, fUnused, fDeltaTicks);
    }
    if (mPointsSlide != nullptr) {
        mPointsSlide->Update(fDelta, false);
    }
}

void OvyScore::Draw() {
    if (mMesh != nullptr) {
        mMesh->Draw();
    }
}

void OvyScore::ShowCheckpoint(float fShare, int nPlace) {
    const float fNow = TheGameDb->GetSongTick();
    mState = kStateMoveIn;
    mMove.Reset(0.0f, 1.0f, fNow, fNow + sMoveInTicks);
    const float fFrames = fShare * kBarFramesPerShare;
    mBarGrow.SetSpeed(fFrames / sBarGrowTicks);
    mBarGrow.Jump(0.0f, fFrames);
    mBarGrow.Update(0.0f, true);
    mCheckpointPos = &sCheckpointPositions[nPlace];
    mStableEnd = mMove.GetX1() + sBarGrowTicks + sStableTicks;
    Color color = mBarMat->mEdgeColor;
    color.a = 1.0f;
    mBarMat->SetEdgeColor(color);
    mBarMat->SetAlpha(color.a);
    if (mPowerup != nullptr) {
        mPowerup->mEnabled = 0;
        mPowerup->UpdateSlide();
    }
    if (mPointsSlide != nullptr) {
        mPointsSlide->SetTarget(kPointsHiddenFrame);
    }
}

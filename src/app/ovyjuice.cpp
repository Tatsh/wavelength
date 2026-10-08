#include "app/ovyjuice.h"

#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "rnd/manager.h"

namespace {

// The fade of a ghost that has long ended, between two song times long past.
constexpr float kLongAgoStart = -1e9f;
constexpr float kLongAgoEnd = -999999872.0f;

// The frames of the level and colour animations per unit.
constexpr float kFramesPerLevel = 1000.0f;
constexpr int kFramesPerColor = 500;

// The polls Reset() runs to settle the display.
constexpr int kSettlePolls = 100;

// The level of a bar after Reset().
constexpr float kNoLevel = -1.0f;

} // namespace

float OvyJuice::sGhostFadeTime = 250.0f;
float OvyJuice::sWarningBlinkTime = 50.0f;
LinearInterpolator OvyJuice::sBlinkRate(1.0f, 1.0f, 0.0f, 0.15f);
float OvyJuice::sBlinkLeft = 0.0f;

OvyJuice::OvyJuice()
    : HideablePanel("HUD1 energy.tnm", "HUD energy.view", false),
      mGhostFade(0.0f, 0.0f, kLongAgoStart, kLongAgoEnd) {
    mView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("HUD energy.view"));
    mBar = dynamic_cast<Rnd::Drawable *>(Rnd::TheManager.Find("HUD energy bar draw.view"));
    mGhost = dynamic_cast<Rnd::Drawable *>(Rnd::TheManager.Find("HUD energy ghost.mesh"));
    mGhostAnim = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("HUD energy ghost.msnm"));
    mGhostFadeAnim =
        dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("HUD energy ghost fade.mnm"));
    mColorAnim = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("HUD energy.mnm"));
    mWarning = dynamic_cast<Rnd::Drawable *>(Rnd::TheManager.Find("HUD ! bg.mesh"));
    Reset();
}

void OvyJuice::Show(bool bShow) {
    bool bShown = false;
    if (bShow && TheGfxManager.mVictoryLap == 0 &&
        TheGameDb->mCommunity == GameDb::kCommunitySolo &&
        TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        bShown = true;
    }
    HideablePanel::Show(bShown);
}

void OvyJuice::Reset() {
    mWarningLeft = 0.0f;
    mWarn = 0;
    mLevel = kNoLevel;
    mGhostFade.Reset(mWarningLeft, mWarningLeft, kLongAgoStart, kLongAgoEnd);
    mWarning->SetShowing(false);
    mGhost->SetShowing(false);
    mBar->SetShowing(false);
    mHideBar = 0;
    for (int i = 0; i < kSettlePolls; ++i) {
        Poll(0.0f);
    }
}

void OvyJuice::SetLevel(float fLevel, int nColor) {
    float fClamped;
    if (1.0f < fLevel) {
        fClamped = 1.0f;
    } else if (fLevel < 0.0f) {
        fClamped = 0.0f;
    } else {
        fClamped = fLevel;
    }
    if (fClamped < mLevel) {
        const float fNow = TheGameDb->mSongTime;
        mGhostFade.Reset(0.0f, kFramesPerLevel, fNow, fNow + sGhostFadeTime);
        mGhostAnim->SetFrame(mLevel * kFramesPerLevel);
    }
    mLevel = fClamped;
    mColorAnim->SetFrame(static_cast<float>(nColor * kFramesPerColor));
}

void OvyJuice::SetWarning(bool bWarn) {
    if (static_cast<int>(bWarn) == mWarn) {
        return;
    }
    mWarn = bWarn;
    mWarning->SetShowing(bWarn);
    mWarningLeft = sWarningBlinkTime;
}

void OvyJuice::Poll(float fDelta) {
    HideablePanel::Poll();
    const float fNow = TheGameDb->mSongTime;
    if (mWarn != 0) {
        mWarningLeft -= fDelta;
        if (mWarningLeft < 0.0f) {
            mWarningLeft += sWarningBlinkTime;
            if (mWarningLeft < 0.0f) {
                mWarningLeft = sWarningBlinkTime;
            }
            mWarning->SetShowing(mWarning->mShowing ^ 1);
        }
    }
    if (mGhostFade.mX1 <= fNow) {
        mGhost->SetShowing(false);
    } else {
        mGhostFadeAnim->SetFrame(mGhostFade.LinearInterpolator::Interp(fNow));
        mGhost->SetShowing(true);
    }
    mView->SetFrame(mLevel * kFramesPerLevel);
    if (mHideBar != 0) {
        mBar->SetShowing(false);
        return;
    }
    if (mLevel <= sBlinkRate.mX1) {
        sBlinkLeft -= fDelta * sBlinkRate.Interpolator::Eval(mLevel);
        if (sBlinkLeft < 0.0f) {
            do {
                sBlinkLeft += 1.0f;
                mBar->SetShowing(mBar->mShowing ^ 1);
                if (sBlinkLeft < 0.0f) {
                    sBlinkLeft = 0.0f;
                }
            } while (sBlinkLeft < 0.0f);
        }
    } else {
        sBlinkLeft = 0.0f;
        mBar->SetShowing(true);
    }
}

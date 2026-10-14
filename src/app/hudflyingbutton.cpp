#include "app/hudflyingbutton.h"

#include "game/gamedb.h"
#include "math/sine.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/rndfont.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmat.h"

namespace {

// The value of mStartTime when no flight runs.
constexpr float kNoFlight = -1e9f;

// The glyphs of the button font, one per lane.
constexpr char kLaneGlyphs[][2] = {"a", "b", "c", "e", "g", "h"};

// The lane whose icon reads the shared flight curve.
constexpr int kCurveLane = 0;

// The pulse runs at 0.002 turns per tick, starting at full brightness.
constexpr float kPi = 3.1415927f;
constexpr float kHalfPi = 1.5707964f;
constexpr float kPulseTurnsPerTick = 0.002f;

void Place(RndText *pText, float fX, float fZ) {
    pText->mDirty = 1;
    // The binary also copies an unset fourth word, which is left as it was here.
    pText->mLocalXfm.mTranslation.x = fX;
    pText->mLocalXfm.mTranslation.y = 0.0f;
    pText->mLocalXfm.mTranslation.z = fZ;
}

} // namespace

float HudFlyingButton::sFlightTime = 0.0f;
Interpolator *HudFlyingButton::sFlight = nullptr;

HudFlyingButton::HudFlyingButton(int nLane, [[maybe_unused]] RndView *pHudView) {
    mText =
        dynamic_cast<RndText *>(TheManager.Find(FormatString("HUD flying button %d.txt", nLane)));
    mText->SetText(kLaneGlyphs[nLane]);
    mBlur =
        dynamic_cast<RndBlur *>(TheManager.Find(FormatString("HUD flying button %d.blur", nLane)));
    if (nLane == kCurveLane) {
        DataArray *pCurve =
            SystemConfig()->FindArray("gfx", true)->FindArray("button_icon_interp", true);
        sFlight = ObjectToInterpolator(pCurve->Array(1));
        sFlightTime = sFlight->mX1;
    }
    Reset();
}

HudFlyingButton::~HudFlyingButton() {
    delete sFlight;
    sFlight = nullptr;
}

void HudFlyingButton::Reset() {
    mBlur->mXfms.clear();
    mBlur->SetShowing(false);
    SetPulse(false);
    mStartTime = kNoFlight;
}

void HudFlyingButton::Fly(const Vector2 *pFrom, const Vector2 *pTo) {
    mFrom = *pFrom;
    mTo = *pTo;
    if (pFrom->x == pTo->x && pFrom->y == pTo->y) {
        Place(mText, mTo.x, mTo.y);
        mStartTime = kNoFlight;
        SetPulse(true);
    } else {
        mStartTime = TheGameDb->mSongTime;
        SetPulse(false);
    }
    mBlur->mXfms.clear();
    mBlur->SetShowing(true);
}

void HudFlyingButton::Hide() {
    mBlur->SetShowing(false);
    SetPulse(false);
}

void HudFlyingButton::SetPulse(bool bPulse) {
    if (!bPulse) {
        mText->mFont->mMat->SetAlpha(1.0f);
    }
    mPulse = bPulse;
}

void HudFlyingButton::Poll([[maybe_unused]] float fRealDelta) {
    if (mText->mShowing == 0 || mStartTime == kNoFlight) {
        return;
    }
    const float fElapsed = TheGameDb->mSongTime - mStartTime;
    if (sFlight->mX1 <= fElapsed) {
        Place(mText, mTo.x, mTo.y);
        mStartTime = kNoFlight;
        SetPulse(true);
        return;
    }
    const float fProgress = sFlight->Interp(fElapsed);
    Place(mText, (mTo.x - mFrom.x) * fProgress + mFrom.x, (mTo.y - mFrom.y) * fProgress + mFrom.y);
}

void HudFlyingButton::Draw() {
    if (mBlur->mShowing == 0) {
        return;
    }
    if (mPulse != 0) {
        const float fTick = TheGameDb->mSongTick;
        const float fWave = FastSin((fTick + fTick) * kPi * kPulseTurnsPerTick + kHalfPi);
        mText->mFont->mMat->SetAlpha(fWave * 0.5f + 0.5f);
    } else {
        mText->mFont->mMat->SetAlpha(1.0f);
    }
    mBlur->Draw();
}

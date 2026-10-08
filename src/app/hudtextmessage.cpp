#include "app/hudtextmessage.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "os/string.h"
#include "rnd/font.h"
#include "rnd/manager.h"

namespace {

// The rows of a transform: three of the basis, then the translation.
enum XfmRow { kXfmRowBasisX = 0, kXfmRowBasisY = 1, kXfmRowBasisZ = 2, kXfmRowTranslation = 3 };

// Scale the basis of a text's local transform by the same factor along x and z.
void SetScale(Rnd::Text *pText, float fScale) {
    float (&xfm)[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount] = pText->mLocalXfm;
    xfm[kXfmRowBasisX][0] = fScale;
    xfm[kXfmRowBasisX][1] = 0.0f;
    xfm[kXfmRowBasisX][2] = 0.0f;
    xfm[kXfmRowBasisY][0] = 0.0f;
    xfm[kXfmRowBasisY][1] = 1.0f;
    xfm[kXfmRowBasisY][2] = 0.0f;
    xfm[kXfmRowBasisZ][0] = 0.0f;
    xfm[kXfmRowBasisZ][1] = 0.0f;
    xfm[kXfmRowBasisZ][2] = fScale;
    pText->mDirty = 1;
}

} // namespace

HudTextMessage::HudTextMessage(Rnd::View *pHudView) {
    mFrame = 0.0f;
    mShowing = 0;
    mStartTime = kNotShown;
    mBlur = dynamic_cast<Rnd::Blur *>(Rnd::TheManager.Find("HUD textmsg.blur"));
    mText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("HUD textmsg.txt"));
    mSmallBlur = dynamic_cast<Rnd::Blur *>(Rnd::TheManager.Find("HUD sm textmsg.blur"));
    mSmallText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("HUD sm textmsg.txt"));
    mMat = mText->mFont->mMat;
    mFlight = dynamic_cast<Rnd::TransAnim *>(
        Rnd::TheManager.Find(FormatString("%s textmsg.tnm", Overlay::sHudPrefix)));
    mOffsetView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("HUD textmsg offset.view"));
    mDefaultColor = mMat->mAmbient;
    mText->SetShowing(false);
    mSmallText->SetShowing(false);
    mBlur->SetShowing(true);
    mSmallBlur->SetShowing(true);
    Rnd::Drawable *pHudDrawable = pHudView;
    pHudDrawable->RemoveDraw(mBlur);
    pHudDrawable->RemoveDraw(mSmallBlur);
}

HudTextMessage::~HudTextMessage() {
}

void HudTextMessage::Hide() {
    mText->SetShowing(false);
    mSmallText->SetShowing(false);
    mShowing = 0;
}

void HudTextMessage::Show(const char *pszText,
                          const char *pszSmallText,
                          float fDuration,
                          float fScale,
                          int nPlayer,
                          float fX,
                          float fZ) {
    if (mShowing != 0) {
        return;
    }
    SetScale(mText, fScale);
    SetScale(mSmallText, fScale);
    mBlur->mXfms.clear();
    mSmallBlur->mXfms.clear();
    mText->SetText(pszText);
    mSmallText->SetText(pszSmallText);
    mText->SetShowing(true);
    mSmallText->SetShowing(true);
    if (nPlayer < 0) {
        mMat->SetAmbient(mDefaultColor);
    } else {
        mMat->SetAmbient(*TheGfxManager.GetPlayerColor(nPlayer));
    }
    Rnd::Transformable *pOffset = mOffsetView;
    pOffset->mLocalXfm[kXfmRowTranslation][0] = fX;
    pOffset->mLocalXfm[kXfmRowTranslation][1] = 0.0f;
    pOffset->mLocalXfm[kXfmRowTranslation][2] = fZ;
    pOffset->mDirty = 1;
    mDuration = fDuration;
    mStartTime = TheGameDb->mSongTime;
    mFlight->SetFrame(0.0f);
}

void HudTextMessage::Poll() {
    if (mStartTime == kNotShown) {
        return;
    }
    const float fElapsed = TheGameDb->mSongTime - mStartTime;
    if (fElapsed < kFlyTime) {
        mFrame = fElapsed;
    } else if (fElapsed < mDuration) {
        mFrame = kFlyTime;
    } else {
        mFrame = fElapsed - mDuration + kFlyTime;
    }
    if (kFlyTime < fElapsed - mDuration) {
        mShowing = 0;
        mStartTime = kNotShown;
        mText->SetShowing(false);
        mSmallText->SetShowing(false);
    }
}

void HudTextMessage::Draw() {
    if (mText->mShowing != 0) {
        mFlight->SetFrame(mFrame);
    }
    mBlur->Draw();
    mSmallBlur->Draw();
}

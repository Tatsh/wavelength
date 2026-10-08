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

// The number of rows of the basis of a transform.
constexpr int kBasisRowCount = 3;

// Copy one row of a transform, including its fourth word.
void CopyRow(float (&destination)[Rnd::kXfmRowFloatCount],
             const float (&source)[Rnd::kXfmRowFloatCount]) {
    for (int i = 0; i < Rnd::kXfmRowFloatCount; ++i) {
        destination[i] = source[i];
    }
}

// Set the basis of a text's local transform.
void SetBasis(Rnd::Text *pText, const float (&basis)[kBasisRowCount][Rnd::kXfmRowFloatCount]) {
    CopyRow(pText->mLocalXfm[kXfmRowBasisX], basis[kXfmRowBasisX]);
    CopyRow(pText->mLocalXfm[kXfmRowBasisY], basis[kXfmRowBasisY]);
    pText->mDirty = 1; // Yes, the binary marks the transform before the last row is written.
    CopyRow(pText->mLocalXfm[kXfmRowBasisZ], basis[kXfmRowBasisZ]);
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
    // Yes, the binary copies the unset fourth words of the basis and of the translation.
    float basis[kBasisRowCount][Rnd::kXfmRowFloatCount];
    basis[kXfmRowBasisX][0] = fScale;
    basis[kXfmRowBasisX][1] = 0.0f;
    basis[kXfmRowBasisX][2] = 0.0f;
    basis[kXfmRowBasisY][0] = 0.0f;
    basis[kXfmRowBasisY][1] = 1.0f;
    basis[kXfmRowBasisY][2] = 0.0f;
    basis[kXfmRowBasisZ][0] = 0.0f;
    basis[kXfmRowBasisZ][1] = 0.0f;
    basis[kXfmRowBasisZ][2] = fScale;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    SetBasis(mText, basis);
    SetBasis(mSmallText, basis);
#pragma GCC diagnostic pop
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
    float translation[Rnd::kXfmRowFloatCount];
    translation[0] = fX;
    translation[1] = 0.0f;
    translation[2] = fZ;
    mOffsetView->mDirty = 1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    CopyRow(mOffsetView->mLocalXfm[kXfmRowTranslation], translation);
#pragma GCC diagnostic pop
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

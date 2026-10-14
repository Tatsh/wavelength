#include "app/hudtextmessage.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "os/string.h"
#include "rnd/rndfont.h"
#include "rnd/rndmanager.h"

namespace {

// The rows of a basis.
enum XfmRow { kXfmRowBasisX = 0, kXfmRowBasisY = 1, kXfmRowBasisZ = 2 };

// The words of a row: three components and the padding.
constexpr int kRowFloatCount = 4;

// The number of rows of the basis of a transform.
constexpr int kBasisRowCount = 3;

// Copy one row of a transform, including its fourth word.
void CopyRow(Vector3 &destination, const float (&source)[kRowFloatCount]) {
    destination.x = source[0];
    destination.y = source[1];
    destination.z = source[2];
    destination.w = source[3];
}

// Set the basis of a text's local transform.
void SetBasis(RndText *pText, const float (&basis)[kBasisRowCount][kRowFloatCount]) {
    CopyRow(pText->mLocalXfm.mBasisX, basis[kXfmRowBasisX]);
    CopyRow(pText->mLocalXfm.mBasisY, basis[kXfmRowBasisY]);
    pText->mDirty = 1; // Yes, the binary marks the transform before the last row is written.
    CopyRow(pText->mLocalXfm.mBasisZ, basis[kXfmRowBasisZ]);
}

} // namespace

HudTextMessage::HudTextMessage(RndView *pHudView) {
    mFrame = 0.0f;
    mShowing = 0;
    mStartTime = kNotShown;
    mBlur = dynamic_cast<RndBlur *>(TheManager.Find("HUD textmsg.blur"));
    mText = dynamic_cast<RndText *>(TheManager.Find("HUD textmsg.txt"));
    mSmallBlur = dynamic_cast<RndBlur *>(TheManager.Find("HUD sm textmsg.blur"));
    mSmallText = dynamic_cast<RndText *>(TheManager.Find("HUD sm textmsg.txt"));
    mMat = mText->mFont->mMat;
    mFlight = dynamic_cast<RndTransAnim *>(
        TheManager.Find(FormatString("%s textmsg.tnm", Overlay::sHudPrefix)));
    mOffsetView = dynamic_cast<RndView *>(TheManager.Find("HUD textmsg offset.view"));
    mDefaultColor = mMat->mBaseColor;
    mText->SetShowing(false);
    mSmallText->SetShowing(false);
    mBlur->SetShowing(true);
    mSmallBlur->SetShowing(true);
    RndDrawable *pHudDrawable = pHudView;
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
    // Yes, the binary copies the unset fourth words of the basis.
    float basis[kBasisRowCount][kRowFloatCount];
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
        mMat->SetBaseColor(mDefaultColor);
    } else {
        mMat->SetBaseColor(*TheGfxManager.GetPlayerColor(nPlayer));
    }
    mOffsetView->mDirty = 1;
    // The binary also copies an unset fourth word, which is left as it was here.
    mOffsetView->mLocalXfm.mTranslation.x = fX;
    mOffsetView->mLocalXfm.mTranslation.y = 0.0f;
    mOffsetView->mLocalXfm.mTranslation.z = fZ;
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

#include "app/ovyduelscore.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

// The value of mFlashTime when no flash runs.
constexpr float kNoFlash = -1e9f;

// The time newly lit letters flash for, in milliseconds.
constexpr float kFlashTime = 960.0f;

} // namespace

OvyDuelScore::OvyDuelScore(int nIndex) : HideablePanel(nullptr, nullptr, false) {
    mLitCount = 0;
    mFlashTime = kNoFlash;
    mView = dynamic_cast<Rnd::View *>(
        Rnd::TheManager.Find(FormatString("HUDd letters %d.view", nIndex)));
    SetObjects(FormatString("HUDd letters %d.tnm", nIndex),
               static_cast<Rnd::Drawable *>(mView)->mName.mStr,
               false);
    mFlashAnim = dynamic_cast<Rnd::Animatable *>(
        Rnd::TheManager.Find(FormatString("HUDd letter hi %d.mnm", nIndex)));
    mFonts[kLetterFontNormal] =
        dynamic_cast<Rnd::Font *>(Rnd::TheManager.Find("HUDd letter no.font"));
    mFonts[kLetterFontNew] = dynamic_cast<Rnd::Font *>(
        Rnd::TheManager.Find(FormatString("HUDd letter hi %d.font", nIndex)));
    for (int i = 0; i < kNumLetters; ++i) {
        mLetters[i] = dynamic_cast<Rnd::Text *>(
            Rnd::TheManager.Find(FormatString("HUDd letter%d %d.txt", i, nIndex)));
    }
    Reset();
}

void OvyDuelScore::SetLitCount(int nCount) {
    ShowLetters(nCount, true);
    mLitCount = nCount;
}

void OvyDuelScore::Poll() {
    HideablePanel::Poll();
    if (mFlashTime == kNoFlash) {
        return;
    }
    const float fElapsed = TheGameDb->mSongTime - mFlashTime;
    if (!(0.0f <= fElapsed)) {
        mFlashAnim->SetFrame(0.0f);
        return;
    }
    if (!(kFlashTime <= fElapsed)) {
        mFlashAnim->SetFrame(fElapsed);
        return;
    }
    mFlashTime = kNoFlash;
    for (Rnd::Text *pLetter : mLetters) {
        pLetter->SetFont(mFonts[kLetterFontNormal]);
    }
}

void OvyDuelScore::Reset() {
    ShowLetters(0, true);
    Show(true);
}

void OvyDuelScore::ShowLetters(int nCount, bool bSetFonts) {
    for (int i = 0; i < kNumLetters; ++i) {
        Rnd::Text *pLetter = mLetters[i];
        pLetter->SetShowing(i < nCount);
        if (!bSetFonts) {
            continue;
        }
        Rnd::Font *pFont = mFonts[i < mLitCount ? kLetterFontNormal : kLetterFontNew];
        if (pFont != pLetter->mFont) {
            pLetter->SetFont(pFont);
        }
    }
    if (bSetFonts && mLitCount < nCount) {
        mFlashTime =
            (TheGameDb->mSongTick + Overlay::sDuelPointsMoveTicks) * *TheGfxManager.mMsPerTick;
    }
}

#include "met/bossunlockscreen.h"

#include "game/gamedb.h"
#include "game/songentry.h"
#include "gfx/gfxmanager.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/animatable.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanel[] = "d_launch";

constexpr float kGoDelayMs = 500.0f;

// The value of mGoMs while the screen does not move on.
constexpr float kNever = 1.0e30f;

} // namespace

BossUnlockScreen::BossUnlockScreen(DataArray *pData)
    : FreqScreen(pData), mGoMs(0.0f), mStarted(0), mSong(nullptr) {
}

void BossUnlockScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mGoMs = kNever;
    FreqScreen::Enter(pPrevScreen, fTime);

    UILabel *pLabel = dynamic_cast<UILabel *>(TheUI.FindComponent(kPanel, "01", false));
    pLabel->SetText(TheLocale.Localize("unlock_boss_1", true));
    pLabel->mText->UpdateCursors();

    pLabel = dynamic_cast<UILabel *>(TheUI.FindComponent(kPanel, "02", false));
    pLabel->SetText(TheLocale.Localize("unlock_boss_2", true));
    pLabel->mText->UpdateCursors();

    pLabel = dynamic_cast<UILabel *>(TheUI.FindComponent(kPanel, "03", false));
    SongEntry entry{TheGameDb->FindSong(mSong)};
    pLabel->SetText(FormatString(TheLocale.Localize("unlock_boss_3", true), entry.GetTitle()));
    pLabel->mText->UpdateCursors();

    mAnimPlayer.SetAnim(dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("d_launch.view")));
}

void BossUnlockScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    mAnimPlayer.Stop();
}

void BossUnlockScreen::Poll(float fTime) {
    if (mStarted && !mAnimPlayer.IsPlaying()) {
        mStarted = 0;
        mGoMs = SystemMs() + kGoDelayMs;
    }
    if (mGoMs < SystemMs()) {
        TheUI.GotoScreen("boss_journey");
        mGoMs = kNever;
    }
    UIScreen::Poll(fTime);
    mAnimPlayer.Poll(fTime);
}

bool BossUnlockScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    TheGfxManager.StartBossJourney();
    mAnimPlayer.Start(TheUI.mTime);
    mStarted = 1;
    return FreqScreen::HandleTransitionComplete(pMsg);
}

bool BossUnlockScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

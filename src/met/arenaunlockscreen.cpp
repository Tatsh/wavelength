#include "met/arenaunlockscreen.h"

#include <vector>

#include "game/gamedb.h"
#include "met/metagame.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/animatable.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"

ArenaUnlockScreen::ArenaUnlockScreen(DataArray *pData)
    : FreqScreen(pData), mStartMs(0.0f), mCount(0), mGoMs(0.0f) {
    pData->FindFloat("hold_time", &mHoldMs, false);
}

void ArenaUnlockScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    std::vector<const char *> arenas;
    TheGameDb->GetUnlockedArenas(&arenas, TheGameDb->mSkillLevel, true, false);
    mCount = static_cast<int>(arenas.size());
    UILabel *pLabel = dynamic_cast<UILabel *>(TheUI.FindComponent("s_unlock_arena", "03", false));
    const String arena(TheMetagame.ArenaName(arenas[mCount - 1]));
    pLabel->SetText(FormatString(TheLocale.Localize("unlock_arena", true), arena.c_str()));
    pLabel->mText->UpdateCursors();
    mAnimPlayer.SetAnim(
        dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("s_unlock_arena.view")));
}

void ArenaUnlockScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    mAnimPlayer.Stop();
}

void ArenaUnlockScreen::Poll(float fTime) {
    if (mStartMs != 0.0f && !mAnimPlayer.IsPlaying()) {
        mStartMs = 0.0f;
        mGoMs = fTime + mHoldMs;
    }
    if (mGoMs != 0.0f && mGoMs < fTime) {
        mGoMs = 0.0f;
        TheUI.GotoScreen(FormatString("unlockarena2anim_0%d", mCount));
    }
    UIScreen::Poll(fTime);
    mAnimPlayer.Poll(fTime);
}

bool ArenaUnlockScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    mStartMs = TheUI.mTime;
    mAnimPlayer.Start(TheUI.mTime);
    return FreqScreen::HandleTransitionComplete(pMsg);
}

bool ArenaUnlockScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

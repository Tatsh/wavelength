#include "met/soloendgamescreen.h"

#include <string.h>

#include "game/avatarpartset.h"
#include "game/avatarplayer.h"
#include "game/campaign.h"
#include "game/gamedb.h"
#include "game/songentry.h"
#include "math/rand.h"
#include "met/avatarpanel.h"
#include "met/metagame.h"
#include "met/sologamestatspanel.h"
#include "met/solowinstatspanel.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/movie.h"
#include "ui/uicomponent.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kWinStatsPanel[] = "s_g_end_win_stats";
constexpr char kContinueButton[] = "continue";
constexpr char kReplayButton[] = "replay";
constexpr char kExitButton[] = "exit";

// The dance of the Freq is one of four loops.
constexpr int kFirstDance = 1;
constexpr int kLastDance = 4;

// The fraction of the song a won game played.
constexpr float kWholeSong = 1.0f;

// Passed through to the avatar player. The meaning is not recovered.
constexpr int kDanceAnimFlags = 1;

} // namespace

SoloEndGameScreen::SoloEndGameScreen(DataArray *pData) : FreqScreen(pData) {
    mAvatarPanel = "";
    mStatsPanel = "";
    pData->FindSymbol("stats_panel", &mStatsPanel, false);
    pData->FindSymbol("av_panel", &mAvatarPanel, false);
}

void SoloEndGameScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    if (strcmp(kWinStatsPanel, mStatsPanel) == 0) {
        Campaign *pProfile = TheGameDb->GetProfile(0);
        const bool bBeatenGame = pProfile->HasBeatenGame(TheGameDb->mSkillLevel);
        const bool bFinished =
            pProfile->IsSongFinished(TheGameDb->mSong.c_str(), TheGameDb->mSkillLevel);
        TheMetagame.RecordCampaignResult();
        UIPanel *pPanel;
        if (TheGameDb->mLoadRemix || TheGameDb->IsWinSequence() || bFinished || bBeatenGame ||
            SongEntry{TheGameDb->FindSong(TheGameDb->mSong.c_str())}.GetType() ==
                SongEntry::kTypeBonus) {
            pPanel = TheUI.FindPanel("s_g_end_win", false);
            pPanel->FindComponent(kContinueButton, false)
                ->SetState(UIComponent::kStateDisabled, false);
            pPanel->SetFocus(pPanel->FindComponent(kExitButton, false), kPadNone);
        } else {
            pPanel = TheUI.FindPanel("s_g_end_win", false);
            pPanel->SetFocus(pPanel->FindComponent(kContinueButton, false), kPadNone);
        }
    } else {
        UILabel *pReplay =
            dynamic_cast<UILabel *>(TheUI.FindComponent("s_g_end_lose", kReplayButton, false));
        if (TheGameDb->mPracticeMode) {
            pReplay->SetText(TheLocale.Localize("s_g_end_lose_play", true));
        } else {
            pReplay->SetText(TheLocale.Localize("s_g_end_lose_play_again", true));
        }
        mFocusPanel->SetFocus(pReplay, kPadNone);
    }

    if (strcmp(mStatsPanel, "") != 0) {
        dynamic_cast<SoloGameStatsPanel *>(TheUI.FindPanel(mStatsPanel, false))->Refresh();
    }

    AvatarPartSet *pAvatar = TheGameDb->GetAvatar(0);
    const int nDance = RandomInt(kFirstDance, kLastDance);
    if (TheGameDb->GetProgress() == kWholeSong) {
        pAvatar->SetBaseAnim(FormatString("win%d_loop", nDance), kDanceAnimFlags);
    } else {
        pAvatar->SetBaseAnim(FormatString("lose%d_loop", nDance), kDanceAnimFlags);
    }
    if (strcmp(mAvatarPanel, "") != 0) {
        dynamic_cast<AvatarPanel *>(TheUI.FindPanel(mAvatarPanel, false))->SetAvatar(pAvatar);
    }
}

const char *SoloEndGameScreen::Title() {
    return "";
}

bool SoloEndGameScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        const char *pszName = pMsg->mComponent->mName;
        Metagame::DialogAction action;
        if (strcmp(pszName, kContinueButton) == 0) {
            action = Metagame::kDialogActionResume;
        } else if (strcmp(pszName, kReplayButton) == 0) {
            action = Metagame::kDialogActionQuit;
        } else if (strcmp(pszName, "practice") == 0) {
            action = Metagame::kDialogActionPractice;
        } else if (strcmp(pszName, kExitButton) == 0) {
            action = Metagame::kDialogActionEnd;
        } else if (strcmp(pszName, "freestyle") == 0) {
            action = Metagame::kDialogActionContinue;
        } else {
            action = static_cast<Metagame::DialogAction>(0);
        }
        TheMetagame.mDialogAction = action;
        TheMetagame.QueueUnlocks();
        TheMetagame.AdvanceUnlocks();
    }
    return UIScreen::HandleSelect(pMsg);
}

bool SoloEndGameScreen::HandleTransitionComplete([[maybe_unused]] UITransitionCompleteMsg *pMsg) {
    if (strcmp(kWinStatsPanel, mStatsPanel) == 0) {
        dynamic_cast<SoloWinStatsPanel *>(TheUI.FindPanel(kWinStatsPanel, false))
            ->StartAnim(TheUI.mTime);
    }
    return false;
}

bool SoloEndGameScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void SoloEndGameScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    dynamic_cast<Rnd::Movie *>(Rnd::TheManager.Find("freq_winner.mov"))
        ->SetFrame(TheGameDb->mSongTick);
    AvatarPlayer::PollAll(fTime);
}

#include "met/modescreen.h"

#include <vector>

#include "game/campaign.h"
#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/jukeboxscreen.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

ModeScreen::ModeScreen(DataArray *pData) : FreqScreen(pData) {
}

bool ModeScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void ModeScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    if (TheGameDb->mCommunity == GameDb::kCommunityLocal &&
        TheGameDb->GetProfile(0)->mCustom != 0) {
        TheMetagame.mGizmo->SetShowAvatar(false);
    }
    if (TheGameDb->mCommunity != GameDb::kCommunitySolo) {
        return;
    }

    JukeboxScreen *pJukebox =
        dynamic_cast<JukeboxScreen *>(TheUI.FindScreen("jbox_redbook", false));
    if (pJukebox->mShowAllSongs != 0) {
        return;
    }
    std::vector<SongEntry> songs;
    TheGameDb->GetUnlockedSongs(&songs, "", GameDb::kSkillAny, true);
    unsigned int i = 0;
    for (; i < songs.size(); ++i) {
        if (TheGameDb->GetProfile(0)->IsSongFinished(songs[i].GetName(), GameDb::kSkillAny)) {
            break;
        }
    }
    if (i == songs.size()) {
        mFocusPanel->FindComponent("jbox_but", true)->SetState(UIComponent::kStateDisabled, false);
    }
}

bool ModeScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }

    String button(pMsg->mComponent->mName);
    if (button == "game_but") {
        TheGameDb->SetTutorial(0);
        TheGameDb->SetRuleSet(GameDb::kRuleSetGame);
    } else if (button == "duel_but") {
        TheGameDb->SetTutorial(0);
        TheGameDb->SetRuleSet(GameDb::kRuleSetDuel);
        TheGameDb->SetSkillLevel(GameDb::kSkillIntermediate);
        if (TheGameDb->GetProfile(0)->mCustom != 0) {
            Campaign first(*TheGameDb->GetProfile(0));
            TheGameDb->ClearPlayers();
            TheGameDb->AddPlayer(&first);
        } else {
            TheGameDb->ClearPlayers();
            Campaign first;
            first.mName = TheLocale.Localize("player_1", true);
            TheGameDb->AddPlayer(&first);
        }
        Campaign second;
        second.mName = TheLocale.Localize("player_2", true);
        TheGameDb->AddPlayer(&second);
    } else {
        TheGameDb->SetLoadRemix(false);
        TheGameDb->SetTutorial(0);
        TheGameDb->SetRuleSet(GameDb::kRuleSetRemix);
    }
    return UIScreen::HandleSelect(pMsg);
}

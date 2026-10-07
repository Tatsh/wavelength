#include "met/metaskillscreen.h"

#include "game/gamedb.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

MetaSkillScreen::MetaSkillScreen(DataArray *pData) : FreqScreen(pData) {
}

bool MetaSkillScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool MetaSkillScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }

    TheGameDb->SetLoadRemix(false);
    TheGameDb->SetTutorial(0);
    String button(pMsg->mComponent->mName);
    if (button == "novice_but") {
        TheGameDb->SetSkillLevel(GameDb::kSkillNovice);
    } else if (button == "intermediate_but") {
        TheGameDb->SetSkillLevel(GameDb::kSkillIntermediate);
    } else if (button == "advanced_but") {
        TheGameDb->SetSkillLevel(GameDb::kSkillAdvanced);
    } else if (button == "insane_but") {
        TheGameDb->SetSkillLevel(GameDb::kSkillInsane);
    }

    const char *pszNext;
    if (button == "custom_but") {
        TheGameDb->SetLoadRemix(true);
        if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
            pszNext = "load_remix_list_game";
        } else {
            pszNext = "load_remix_list_multi_game";
        }
    } else if (button == "d_tips_but") {
        pszNext = mName;
    } else if (button == "m_tips_but") {
        pszNext = "m_tip_in";
    } else if (button == "pup_but") {
        pszNext = "pup_tip_in";
    } else if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        pszNext = "soloskill2soloarena";
    } else if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        pszNext = "duel2multisong";
    } else {
        pszNext = "m_powerup";
    }
    TheMetagame.ShowUnlockedArenas();
    TheUI.GotoScreen(pszNext);
    return UIScreen::HandleSelect(pMsg);
}

bool MetaSkillScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    if (pMsg->mComponent == nullptr || pMsg->mPanel != mFocusPanel) {
        return false;
    }
    String token;
    token = FormatString("%s_%s_HELP", pMsg->mPanel->mName, pMsg->mComponent->mName);
    TheMetagame.SetHelpText(TheLocale.Localize(token.c_str(), true));
    return false;
}

bool MetaSkillScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (pMsg->mScreen == this) {
        mFocusPanel->SetFocus(mFocusPanel->mFocus, kPadNone);
    }
    return false;
}

void MetaSkillScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    UIPanel *pPanel = TheUI.FindPanel(mFocusPanel->mName, false);
    const char *pszButton = "";
    switch (TheGameDb->mSkillLevel) {
    case GameDb::kSkillNovice:
        pszButton = "novice_but";
        break;
    case GameDb::kSkillIntermediate:
        pszButton = "intermediate_but";
        break;
    case GameDb::kSkillAdvanced:
        pszButton = "advanced_but";
        break;
    case GameDb::kSkillInsane:
        pszButton = "insane_but";
        break;
    default:
        break;
    }
    pPanel->SetFocus(pPanel->FindComponent(pszButton, false), kPadNone);
    FreqScreen::Enter(pPrevScreen, fTime);
}

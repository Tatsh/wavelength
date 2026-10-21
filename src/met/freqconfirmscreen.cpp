#include "met/freqconfirmscreen.h"

#include <string.h>

#include "game/campaign.h"
#include "game/gamedb.h"
#include "met/freqmakermainscreen.h"
#include "met/metagame.h"
#include "met/selloadedfreqscreen.h"
#include "os/cheatsmanager.h"
#include "os/codecheck.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

FreqConfirmScreen::FreqConfirmScreen(DataArray *pData) : FreqScreen(pData) {
}

bool FreqConfirmScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void FreqConfirmScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
#ifdef VIDEO_STANDARD_PAL
    CodeCheck::Poll();
#endif
    UIComponent *pConfirm = TheUI.FindComponent("f_confirm", "confirm", false);
    UIComponent *pEdit = TheUI.FindComponent("f_confirm", "edit", false);
    String name("");

    int nCustom = TheGameDb->GetProfile(0)->GetCustom();
    if (TheGameDb->GetCommunity() == GameDb::kCommunityOnline &&
        CheatsManager::IsCheatEntered() != 0) {
        nCustom = 0;
    }
    if (nCustom != 0) {
        mFocusPanel->SetFocus(pConfirm, kPadNone);
        name = TheGameDb->GetProfile(0)->GetName().c_str();
    } else {
        pConfirm->SetState(UIComponent::kStateDisabled, false);
        pEdit->SetState(UIComponent::kStateDisabled, false);
        UIComponent *pFocus = TheUI.FindComponent(
            "f_confirm", TheMetagame.GetFreqsOnCard() != 0 ? "load" : "create", false);
        mFocusPanel->SetFocus(pFocus, kPadNone);
        pFocus->SetState(UIComponent::kStateSelected, true);
    }

    pConfirm->SetText(FormatString(TheLocale.Localize("f_confirm_01_but", true), name.c_str()));
    pEdit->SetText(FormatString(TheLocale.Localize("f_confirm_02_but", true), name.c_str()));
}

bool FreqConfirmScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->GetButton() != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }

    const char *pszButton = pMsg->GetComponent()->GetName();
    if (strcmp(pszButton, "edit") == 0) {
        FreqMakerMainScreen *pMaker =
            dynamic_cast<FreqMakerMainScreen *>(TheUI.FindScreen("f_maker", false));
        pMaker->SetSaveStarted(0);
        pMaker->SetEditing(1);
        pMaker->SetFreqName(TheGameDb->GetPlayerName(0));
        pMaker->SetProfile(*TheGameDb->GetProfile(0));
    } else if (strcmp(pszButton, "create") == 0) {
        FreqMakerMainScreen *pMaker =
            dynamic_cast<FreqMakerMainScreen *>(TheUI.FindScreen("f_maker", false));
        pMaker->SetEditing(0);
        pMaker->SetSaveStarted(0);
        pMaker->SetFreqName("");
        pMaker->SetProfile(*TheGameDb->GetProfile(0));
    } else if (strcmp(pszButton, "load") == 0) {
        String screen;
        if (TheGameDb->GetCommunity() == GameDb::kCommunityOnline) {
            screen = "f_net_load";
        } else {
            screen = "f_load";
        }
        SelLoadedFreqScreen *pLoad =
            dynamic_cast<SelLoadedFreqScreen *>(TheUI.FindScreen(screen.c_str(), false));
        pLoad->SetProfile(*TheGameDb->GetProfile(0));
    }

    if (strcmp(pszButton, "create") != 0 && strcmp(pszButton, "load") != 0) {
        return UIScreen::HandleSelect(pMsg);
    }

    if (TheGameDb->GetProfile(0)->GetCustom() != 0 && TheGameDb->GetProfile(0)->IsModified() != 0) {
        UIScreen *pScreen = TheUI.FindScreen("cur_freq_not_saved", false);
        pScreen->ClearTransitions();
        const bool bOnline = TheGameDb->GetCommunity() == GameDb::kCommunityOnline;
        if (strcmp(pszButton, "create") == 0) {
            pScreen->AddTransition(
                "save", kPadNone, bOnline ? "pre_f_maker_save_net" : "pre_f_maker_save");
            pScreen->AddTransition("continue", kPadNone, "f_maker");
        } else if (bOnline) {
            pScreen->AddTransition("save", kPadNone, "pre_f_load_save_net");
            pScreen->AddTransition("continue", kPadNone, "net_load_freq");
        } else {
            pScreen->AddTransition("save", kPadNone, "pre_f_load_save");
            pScreen->AddTransition("continue", kPadNone, "load_freq");
        }
        pScreen->AddTransition("cancel", kPadNone, mName);
        TheUI.GotoScreen(pScreen);
        return true;
    }

    if (strcmp(pszButton, "create") == 0) {
        TheGameDb->ClearPlayers();
        Campaign profile;
        profile.SetName(TheLocale.Localize("player_1", true));
        TheGameDb->AddPlayer(&profile);
    }
    return UIScreen::HandleSelect(pMsg);
}

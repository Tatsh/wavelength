#include "met/metamainscreen.h"

#include "game/gamedb.h"
#include "game/playerprofile.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"

namespace {

// The fewest controllers that Enter() reduces to the first player.
constexpr int kMultiplePads = 2;

// The panel whose focus changes play the portal sounds. The panel name is a symbol, and the binary
// compares its address with this literal's.
const char *const kMainPanel = "main";

} // namespace

MetaMainScreen::MetaMainScreen(DataArray *pData) : FreqScreen(pData) {
}

void MetaMainScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    if (TheGameDb->GetNumPads() >= kMultiplePads) {
        PlayerProfile profile(*TheGameDb->GetProfile(0));
        TheGameDb->ClearPlayers();
        TheGameDb->AddPlayer(&profile);
    }
}

void MetaMainScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    FxMidi::StopPortals();
}

bool MetaMainScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool MetaMainScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }

    String button(pMsg->mComponent->mName);
    TheGameDb->SetSkillLevel(GameDb::kSkillIntermediate);
    TheGameDb->SetTutorial(0);
    TheGameDb->SetPracticeMode(false);
    if (button == "solo_but") {
        TheGameDb->SetCommunity(GameDb::kCommunitySolo);
    } else if (button == "multi_but") {
        TheGameDb->SetCommunity(GameDb::kCommunityLocal);
        TheMetagame.mGizmo->SetShowAvatar(false);
        if (TheGameDb->GetProfile(0)->IsModified() != 0) {
            UIScreen *pScreen = TheUI.FindScreen("cur_freq_not_saved", false);
            pScreen->ClearTransitions();
            pScreen->AddTransition("save", kPadNone, "pre_multi_save");
            pScreen->AddTransition("continue", kPadNone, "main2multifreq");
            pScreen->AddTransition("cancel", kPadNone, mName);
            TheUI.GotoScreen(pScreen);
            return true;
        }
    } else if (button == "freqnet_but") {
        TheGameDb->SetCommunity(GameDb::kCommunityOnline);
        TheGameDb->SetTutorial(0);
        TheGameDb->SetRuleSet(GameDb::kRuleSetGame);
    } else if (button == "options_but") {
        TheGameDb->SetCommunity(GameDb::kCommunityNone);
    }

    if (button == "solo_but" || button == "freqnet_but") {
        if (TheGameDb->GetProfile(0)->mCustom != 0) {
            PlayerProfile profile(*TheGameDb->GetProfile(0));
            TheGameDb->ClearPlayers();
            TheGameDb->AddPlayer(&profile);
            TheMetagame.mGizmo->SetShowAvatar(true);
        } else {
            TheGameDb->ClearPlayers();
            PlayerProfile profile;
            profile.mName = TheLocale.Localize("player_1", true);
            TheGameDb->AddPlayer(&profile);
            TheMetagame.mGizmo->SetShowAvatar(false);
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

bool MetaMainScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    if (pMsg->mPanel->mName != kMainPanel) {
        return false;
    }

    FxMidi::StopPortals();
    String button(pMsg->mComponent->mName);
    if (button == "solo_but") {
        FxMidi::PlaySoloPortal();
    } else if (button == "multi_but") {
        FxMidi::PlayMultiPortal();
    } else if (button == "freqnet_but") {
        FxMidi::PlayNetPortal();
    }
    return false;
}

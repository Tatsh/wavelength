#include "met/multisetupremixsavescreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/metagame.h"
#include "met/metagameutil.h"
#include "met/remixdiscardscreen.h"
#include "met/saveremixscreen.h"
#include "met/transitionerrorscreen.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanel[] = "m_r_end";
constexpr char kSaveFormat[] = "m_r_end_remix_%d";
constexpr char kErrorScreen[] = "lpad_error";

// A screen name ends with the digit of one of this many players.
constexpr int kMaxPlayers = 4;
constexpr int kFirstPlayer = 1;

// IsGuest() reports this value for the host.
constexpr int kHostIndex = 0;

#ifdef VIDEO_STANDARD_PAL
constexpr int kThirdPlayer = 3;
constexpr int kFourthPlayer = 4;

constexpr char kMemcardDiscardScreen[] = "mc_remix_discard";

// Have continuing without the save ask the player whether to discard the remix.
inline void UseDiscardScreen(SaveRemixScreen *pSave, int nPlayer) {
    pSave->SetContinueScreen(kMemcardDiscardScreen);
    dynamic_cast<RemixDiscardScreen *>(TheUI.FindScreen(kMemcardDiscardScreen, false))
        ->SetPlayer(nPlayer);
}
#endif

} // namespace

MultiSetupRemixSaveScreen::MultiSetupRemixSaveScreen(DataArray *pData)
    : SetupRemixSaveScreen(pData) {
    const char cDigit = mName[strlen(mName) - 1];
    mPlayer = cDigit - '0';
    if (static_cast<unsigned int>(cDigit - '1') >= kMaxPlayers) {
        mPlayer = kFirstPlayer;
    }
    mFromShare = 0;
}

void MultiSetupRemixSaveScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    SetupRemixSaveScreen::Enter(pPrevScreen, fTime);
    UIComponent *pNumber = TheUI.FindComponent(kPanel, "player_num", false);
    pNumber->SetText(FormatString(TheLocale.Localize("m_r_end_03", true), mPlayer));
    if (TheGameDb->GetCommunity() == GameDb::kCommunityOnline) {
        TheMetagame.SetReserved144(0);
    }
}

void MultiSetupRemixSaveScreen::Proceed() {
    FxMidi::PlayMenuSelect();
    String title(TheUI.FindComponent(kPanel, "title", false)->Text());
    const bool bTrimmed = TrimSpaces(&title);
    if (title.GetLength() == 0 || bTrimmed) {
        const char *pszError =
            title.GetLength() == 0 ? "no_empty_filename_screen" : "no_lead_trail_spaces_screen";
        UIScreen *pError = TheUI.FindScreen(pszError, false);
        pError->ClearTransitions();
        pError->AddTransition("ok", kPadNone, FormatString(kSaveFormat, mPlayer));
        TheUI.GotoScreen(pError);
        return;
    }

    auto *pSave = dynamic_cast<SaveRemixScreen *>(TheUI.FindScreen("save_remix", false));
    pSave->SetStartScreen(mName);
    pSave->SetOverwriteStatus(0);
    if (TheGameDb->GetCommunity() == GameDb::kCommunityLocal) {
        if (mPlayer < TheGameDb->GetNumPlayers()) {
            pSave->SetDoneScreen(FormatString(kSaveFormat, mPlayer + 1));
        } else {
            pSave->SetDoneScreen("m_r_mode");
        }
        pSave->SetSlot(mPlayer - 1);
#ifdef VIDEO_STANDARD_PAL
        UseDiscardScreen(pSave, mPlayer);
#endif
    } else {
        pSave->SetStartScreen("net_end_remix");
#ifdef VIDEO_STANDARD_PAL
        UseDiscardScreen(pSave, mPlayer);
#endif
        if (TheNetLaunchpad == nullptr) {
            if (TheMetagame.GetNetScreenPending() != 0) {
                pSave->SetDoneScreen(TheMetagame.GetNetScreen());
            } else {
                String text(TheLocale.Localize("net_lpad_lost_error_msg", true));
                auto *pError =
                    dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kErrorScreen, false));
                pError->SetMessage(text.c_str());
                pSave->SetDoneScreen(kErrorScreen);
            }
        } else if (mFromShare != 0) {
            pSave->SetDoneScreen("fn_g_lpad");
            mFromShare = 0;
        } else if (TheNetLaunchpad->IsGuest() == kHostIndex) {
            pSave->SetDoneScreen("netlobby2netlaunchpad_host");
        } else {
            pSave->SetDoneScreen("netlobby2netlaunchpad_guest");
        }
    }
    pSave->SetRemixName(title.c_str());
    TheUI.GotoScreen(pSave);
}

bool MultiSetupRemixSaveScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return SetupRemixSaveScreen::DispatchPriv(pMsg);
}

bool MultiSetupRemixSaveScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (strcmp(pMsg->GetPrevScreen()->GetName(), "net_share_remix") == 0) {
        mFromShare = 1;
#ifdef VIDEO_STANDARD_PAL
    } else if ((mPlayer == kThirdPlayer || mPlayer == kFourthPlayer) &&
               !JoypadMultitapConnected()) {
        // The third and fourth players save to the multitap's slots.
        auto *pError =
            dynamic_cast<ErrorScreen *>(TheUI.FindScreen("remix_save_no_mtap_34_error", false));
        pError->SetSlot(mPlayer - 1);
        pError->ClearTransitions();
        pError->AddTransition("no", kPadNone, mName);
        pError->AddTransition("yes", kPadNone, "m_r_mode");
        TheUI.GotoScreen(pError);
#endif
    }
    return SetupRemixSaveScreen::HandleTransitionComplete(pMsg);
}

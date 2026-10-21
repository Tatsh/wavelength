#include "met/loadnetconfigsscreen.h"

#include <cstring>

#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/netchooseiconfigscreen.h"
#include "met/transitionerrorscreen.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uimanager.h"

namespace {

constexpr char kConfigScreen[] = "fn_config";
constexpr char kConfigTransition[] = "netfreq2netconfig";
constexpr char kErrorScreen[] = "net_load_config_error";
constexpr char kNotFoundToken[] = "config_not_found_msg";
constexpr char kWrongConsoleToken[] = "wrong_console_err_msg";
constexpr char kConfigErrorToken[] = "random_config_err_msg";
constexpr char kNoToken[] = "";

#ifndef VIDEO_STANDARD_PAL
// The operation of ErrorScreen::ShowCardErrorTwoOption() for a failure outside a save, a copy, and
// a deletion.
constexpr int kNoOperation = 0;
#endif

// The slot whose name the error message shows.
constexpr int kFirstSlot = 0;

NetChooseIConfigScreen *FindConfigScreen() {
    return dynamic_cast<NetChooseIConfigScreen *>(TheUI.FindScreen(kConfigScreen, false));
}

} // namespace

void LoadNetConfigsScreen::OnNetConfigsListed(int nStatus, std::list<InetConfig> *pConfigs) {
    const char *pszToken = kNoToken;
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        FindConfigScreen()->SetConfigs(*pConfigs);
        TheUI.GotoScreen(kConfigTransition);
        break;
#ifdef VIDEO_STANDARD_PAL
    case MemcardTask::kStatusUnformatted:
#endif
    case MemcardTask::kStatusNotFound:
        pszToken = kNotFoundToken;
        break;
    case MemcardTask::kStatusConfigsBad:
        pszToken = kWrongConsoleToken;
        break;
    case MemcardTask::kStatusConfigsError:
        pszToken = kConfigErrorToken;
        break;
#ifdef VIDEO_STANDARD_PAL
    case MemcardTask::kStatusChangedCard:
        mRelist = 1;
        break;
    default:
        ShowCardErrorTwoOption(nStatus, kOperationLoad);
        break;
#else
    default:
        ShowCardErrorTwoOption(nStatus, kNoOperation);
        break;
#endif
    }
    if (std::strcmp(pszToken, kNoToken) == 0) {
        return;
    }

    const String text(
        FormatString(TheLocale.Localize(pszToken, true), TheMCManager.GetSlotName(kFirstSlot)));
    auto *pError = dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kErrorScreen, false));
    pError->SetMessage(text.c_str());
    TheUI.GotoScreen(pError);
}

bool LoadNetConfigsScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool LoadNetConfigsScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->GetScreen()->GetName(), mName) == 0) {
        const std::list<InetConfig> noConfigs;
        FindConfigScreen()->SetConfigs(noConfigs);
        TheMCManager.ListNetConfigs(this, mSlot);
    }
    return false;
}

#ifdef VIDEO_STANDARD_PAL
void LoadNetConfigsScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mRelist != 0) {
        mRelist = 0;
        const std::list<InetConfig> noConfigs;
        FindConfigScreen()->SetConfigs(noConfigs);
        TheMCManager.ListNetConfigs(this, mSlot);
    }
}
#endif

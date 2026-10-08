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

// The operation of ErrorScreen::ShowCardErrorTwoOption() for a failure outside a save, a copy, and
// a deletion.
constexpr int kNoOperation = 0;

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
    case MemcardTask::kStatusNotFound:
        pszToken = kNotFoundToken;
        break;
    case MemcardTask::kStatusConfigsBad:
        pszToken = kWrongConsoleToken;
        break;
    case MemcardTask::kStatusConfigsError:
        pszToken = kConfigErrorToken;
        break;
    default:
        ShowCardErrorTwoOption(nStatus, kNoOperation);
        break;
    }
    if (std::strcmp(pszToken, kNoToken) == 0) {
        return;
    }

    const String text(
        FormatString(TheLocale.Localize(pszToken, true), TheMCManager.GetSlotName(kFirstSlot)));
    auto *pError = dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kErrorScreen, false));
    pError->mMessage = text.c_str();
    TheUI.GotoScreen(pError);
}

bool LoadNetConfigsScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool LoadNetConfigsScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        const std::list<InetConfig> noConfigs;
        FindConfigScreen()->SetConfigs(noConfigs);
        TheMCManager.ListNetConfigs(this, mSlot);
    }
    return false;
}

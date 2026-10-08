#include "met/netgetdownloadsscreen.h"

#include <cstring>

#include "met/selremixdownloadscreen.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlobby.h"
#include "ui/uimanager.h"

namespace {

constexpr char kDownloadScreen[] = "fn_download";
constexpr char kErrorScreen[] = "net_get_download_list_error";

// The value of RepoRemixesMsg::mResult for a reply.
constexpr int kResultOk = 0;

} // namespace

bool NetGetDownloadsScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nRepoRemixesMsgType) {
        return HandleRepoRemixes(static_cast<RepoRemixesMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetGetDownloadsScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheNetLobby->RequestRemixes(this);
    }
    return false;
}

bool NetGetDownloadsScreen::HandleRepoRemixes(RepoRemixesMsg *pMsg) {
    if (pMsg->mResult == kResultOk && !pMsg->mRemixes->empty()) {
        auto *pScreen =
            dynamic_cast<SelRemixDownloadScreen *>(TheUI.FindScreen(kDownloadScreen, false));
        pScreen->mRemixes = *pMsg->mRemixes;
        TheUI.GotoScreen(pScreen);
    } else {
        TheUI.GotoScreen(kErrorScreen);
    }
    return false;
}

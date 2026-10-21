#include "met/netsavereposlistscreen.h"

#ifdef VIDEO_STANDARD_PAL

#include <cstring>

#include "netflow/netlobby.h"
#include "ui/uimanager.h"

namespace {

constexpr char kUploadToolScreen[] = "fn_upload_tool";
constexpr char kErrorScreen[] = "net_save_repos_list_error";

} // namespace

bool NetSaveReposListScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nRepoRemixStatusMsgType) {
        return HandleRepoRemixStatus(static_cast<RepoRemixStatusMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetSaveReposListScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->GetName(), mName) == 0) {
        TheNetLobby->UploadDirectory(this, &mRemixes);
    }
    return false;
}

bool NetSaveReposListScreen::HandleRepoRemixStatus(RepoRemixStatusMsg *pMsg) {
    if (pMsg->mStatus == RepoRemixStatusMsg::kStatusDone) {
        TheUI.GotoScreen(kUploadToolScreen);
    } else if (pMsg->mStatus == RepoRemixStatusMsg::kStatusFailed) {
        TheUI.GotoScreen(kErrorScreen);
    }
    return false;
}

#endif

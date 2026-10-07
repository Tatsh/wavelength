#include "met/netdodownloadscreen.h"

#include <cstring>

#include "met/dialogpanel.h"
#include "met/saveremixscreen.h"
#include "netflow/netlobby.h"
#include "os/locale.h"
#include "ui/uimanager.h"

namespace {

constexpr char kProgressToken[] = "fn_do_download_dlg";
constexpr char kSaveScreen[] = "save_remix_download";
constexpr char kErrorScreen[] = "fn_download_remix_error";

constexpr float kPercent = 100.0f;
constexpr int kNoOverwrite = 0;

} // namespace

NetDoDownloadScreen::NetDoDownloadScreen(DataArray *pData) : FreqScreen(pData) {
}

void NetDoDownloadScreen::ShowProgress(float fProgress) {
    const char *pszText = FormatString(TheLocale.Localize(kProgressToken, true),
                                       static_cast<int>(fProgress * kPercent));
    dynamic_cast<DialogPanel *>(mFocusPanel)->SetText(pszText);
}

bool NetDoDownloadScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nRepoRemixStatusMsgType) {
        return HandleRepoRemixStatus(static_cast<RepoRemixStatusMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetDoDownloadScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        ShowProgress(0.0f);
        TheNetLobby->DownloadRemix(this, mFile.c_str());
    }
    return false;
}

bool NetDoDownloadScreen::HandleRepoRemixStatus(RepoRemixStatusMsg *pMsg) {
    if (pMsg->mStatus == RepoRemixStatusMsg::kStatusDone) {
        SaveRemixScreen *pSave =
            dynamic_cast<SaveRemixScreen *>(TheUI.FindScreen(kSaveScreen, false));
        pSave->mRemixName = mRemixName.c_str();
        pSave->mOverwriteStatus = kNoOverwrite;
        TheUI.GotoScreen(pSave);
    } else if (pMsg->mStatus == RepoRemixStatusMsg::kStatusProgress) {
        ShowProgress(pMsg->mProgress);
    } else {
        TheUI.GotoScreen(kErrorScreen);
    }
    return false;
}

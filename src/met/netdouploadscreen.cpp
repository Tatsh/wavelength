#include "met/netdouploadscreen.h"

#include <cstring>

#include "met/dialogpanel.h"
#include "netflow/netlobby.h"
#include "os/locale.h"
#include "ui/uimanager.h"

namespace {

constexpr char kProgressToken[] = "fn_do_upload_dlg";
constexpr char kDoneScreen[] = "save_settings_upload";
constexpr char kAlreadyExistsScreen[] = "fn_upload_remix_error_already";
constexpr char kErrorScreen[] = "fn_upload_remix_error";

constexpr float kPercent = 100.0f;

} // namespace

NetDoUploadScreen::NetDoUploadScreen(DataArray *pData) : FreqScreen(pData) {
}

void NetDoUploadScreen::ShowProgress(float fProgress) {
    const char *pszText = FormatString(TheLocale.Localize(kProgressToken, true),
                                       static_cast<int>(fProgress * kPercent));
    dynamic_cast<DialogPanel *>(mFocusPanel)->SetText(pszText);
}

bool NetDoUploadScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nRepoRemixStatusMsgType) {
        return HandleRepoRemixStatus(static_cast<RepoRemixStatusMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetDoUploadScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        ShowProgress(0.0f);
        TheNetLobby->UploadRemix(this, mNote.c_str());
    }
    return false;
}

bool NetDoUploadScreen::HandleRepoRemixStatus(RepoRemixStatusMsg *pMsg) {
    switch (pMsg->mStatus) {
    case RepoRemixStatusMsg::kStatusDone:
        TheUI.GotoScreen(kDoneScreen);
        break;
    case RepoRemixStatusMsg::kStatusProgress:
        ShowProgress(pMsg->mProgress);
        break;
    case RepoRemixStatusMsg::kStatusAlreadyExists:
        TheUI.GotoScreen(kAlreadyExistsScreen);
        break;
    default:
        TheUI.GotoScreen(kErrorScreen);
        break;
    }
    return false;
}

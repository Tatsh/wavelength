#include "met/netgetalluploadsscreen.h"

#ifdef VIDEO_STANDARD_PAL

#include <cstring>

#include "met/netrepostoolscreen.h"
#include "met/netuploadtoolscreen.h"
#include "netflow/netlobby.h"
#include "os/debug.h"
#include "ui/uimanager.h"

namespace {

constexpr char kReposToolScreen[] = "fn_repos_tool";
constexpr char kUploadToolScreen[] = "fn_upload_tool";

constexpr int kOnlyRemixes = 1;
constexpr int kRequestSucceeded = 0;

} // namespace

bool NetGetAllUploadsScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nFileListMsgType) {
        return HandleFileList(static_cast<FileListMsg *>(pMsg));
    }
    if (nType == g_nRepoRemixesMsgType) {
        return HandleRepoRemixes(static_cast<RepoRemixesMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nRepoRemixStatusMsgType) {
        return HandleRepoRemixStatus(static_cast<RepoRemixStatusMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetGetAllUploadsScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheNetLobby->RequestFiles(this, kOnlyRemixes);
    }
    return false;
}

bool NetGetAllUploadsScreen::HandleRepoRemixStatus(RepoRemixStatusMsg *pMsg) {
    pMsg->Print(TheDebug);
    TheDebug << "\n";
    return false;
}

bool NetGetAllUploadsScreen::HandleFileList(FileListMsg *pMsg) {
    mFiles = *pMsg->mFiles;
    TheNetLobby->RequestRemixes(this);
    return false;
}

bool NetGetAllUploadsScreen::HandleRepoRemixes(RepoRemixesMsg *pMsg) {
    std::list<NetRepoRemix> noRemixes;
    std::list<NetRepoRemix> *pRemixes = &noRemixes;
    if (pMsg->mResult == kRequestSucceeded) {
        pRemixes = pMsg->mRemixes;
    }
    dynamic_cast<NetReposToolScreen *>(TheUI.FindScreen(kReposToolScreen, false))->mRemixes =
        *pRemixes;
    auto *pUploadTool =
        dynamic_cast<NetUploadToolScreen *>(TheUI.FindScreen(kUploadToolScreen, false));
    pUploadTool->SetFiles(mFiles, *pRemixes);
    TheUI.GotoScreen(pUploadTool);
    return false;
}

#endif

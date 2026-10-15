#include "met/netdodownloadscreen.h"

#include <cstring>

#include "met/dialogpanel.h"
#include "met/saveremixscreen.h"
#include "netflow/netlobby.h"
#include "os/locale.h"
#include "ui/uimanager.h"

#ifdef VIDEO_STANDARD_PAL
#include "game/gamedb.h"
#include "met/netrepostoolscreen.h"
#include "met/netuploadtoolscreen.h"
#include "netflow/netreporemix.h"
#endif

namespace {

constexpr char kProgressToken[] = "fn_do_download_dlg";
constexpr char kSaveScreen[] = "save_remix_download";
constexpr char kErrorScreen[] = "fn_download_remix_error";

constexpr float kPercent = 100.0f;
constexpr int kNoOverwrite = 0;

#ifdef VIDEO_STANDARD_PAL
constexpr char kDownloadToolScreen[] = "fn_do_download_tool";
constexpr char kPreviewRemixScreen[] = "fn_do_download_preview_remix";
constexpr char kPreviewGameScreen[] = "fn_do_download_preview_game";
constexpr char kUploadToolScreen[] = "fn_upload_tool";
constexpr char kReposToolScreen[] = "fn_repos_tool";
constexpr char kUnplayableScreen[] = "preview_game_unplayable";
constexpr char kLaunchScreen[] = "pre_net_tool2launchseq";

constexpr int kDownloadSucceeded = 0;
constexpr int kReadOnly = 1;
constexpr int kNotTutorial = 0;
constexpr int kRemixActive = 1;

// Set up a solo game of the downloaded remix and go to the launch sequence.
inline void StartPreview(const RemixInfo &info, int nRuleSet, bool bPracticeMode, int nReadOnly) {
    TheGameDb->SetCommunity(GameDb::kCommunitySolo);
    TheGameDb->SetRuleSet(nRuleSet);
    TheGameDb->SetSong(info.mSong);
    TheGameDb->SetPracticeMode(bPracticeMode);
    TheGameDb->SetTutorial(kNotTutorial);
    TheGameDb->SetRemixActive(kRemixActive);
    TheGameDb->SetLoadRemix(true);
    TheGameDb->SetSkillLevel(info.mSkillLevel);
    TheGameDb->SetRemixName(info.mName);
    TheGameDb->SetRemixReadOnly(nReadOnly);
    TheUI.GotoScreen(kLaunchScreen);
}
#endif

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
#ifdef VIDEO_STANDARD_PAL
    if (nType == g_nRemixDownloadMsgType) {
        return HandleRemixDownload(static_cast<RemixDownloadMsg *>(pMsg));
    }
#endif
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

#ifdef VIDEO_STANDARD_PAL
bool NetDoDownloadScreen::HandleRepoRemixStatus(RepoRemixStatusMsg *pMsg) {
    if (pMsg->mStatus == RepoRemixStatusMsg::kStatusProgress) {
        ShowProgress(pMsg->mProgress);
    }
    if (std::strcmp(mName, kDownloadToolScreen) == 0 ||
        std::strcmp(mName, kPreviewRemixScreen) == 0 ||
        std::strcmp(mName, kPreviewGameScreen) == 0) {
        return false;
    }
    // Yes, the binary sends a progress report of the other screens to the error screen as well.
    if (pMsg->mStatus == RepoRemixStatusMsg::kStatusDone) {
        SaveRemixScreen *pSave =
            dynamic_cast<SaveRemixScreen *>(TheUI.FindScreen(kSaveScreen, false));
        pSave->mRemixName = mRemixName.c_str();
        pSave->mOverwriteStatus = kNoOverwrite;
        TheUI.GotoScreen(pSave);
    } else {
        TheUI.GotoScreen(kErrorScreen);
    }
    return false;
}

bool NetDoDownloadScreen::HandleRemixDownload(RemixDownloadMsg *pMsg) {
    if (pMsg->mResult != kDownloadSucceeded) {
        auto *pUploadTool =
            dynamic_cast<NetUploadToolScreen *>(TheUI.FindScreen(kUploadToolScreen, false));
        if (mUploadIndex != kNoUploadIndex) {
            pUploadTool->RemoveFile(mUploadIndex);
        }
        mUploadIndex = kNoUploadIndex;
        TheUI.GotoScreen(kUploadToolScreen);
        return false;
    }
    if (std::strcmp(mName, kDownloadToolScreen) == 0) {
        auto *pRepos =
            dynamic_cast<NetReposToolScreen *>(TheUI.FindScreen(kReposToolScreen, false));
        NetRepoRemix remix;
        remix.mFile = pMsg->mName;
        remix.mNote = pMsg->mNote;
        remix.mInfo = *TheGameDb->GetRemixInfo();
        pRepos->AddRemix(remix);
        TheUI.GotoScreen(kReposToolScreen);
    } else if (std::strcmp(mName, kPreviewGameScreen) == 0) {
        const RemixInfo info = *TheGameDb->GetRemixInfo();
        if (info.mPlayable == 0) {
            TheUI.GotoScreen(kUnplayableScreen);
        } else {
            StartPreview(info, GameDb::kRuleSetGame, true, info.mReadOnly != 0);
        }
    } else if (std::strcmp(mName, kPreviewRemixScreen) == 0) {
        const RemixInfo info = *TheGameDb->GetRemixInfo();
        StartPreview(info, GameDb::kRuleSetRemix, false, kReadOnly);
    }
    return false;
}
#else
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
#endif

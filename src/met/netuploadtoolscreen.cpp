#include "met/netuploadtoolscreen.h"

#ifdef VIDEO_STANDARD_PAL

#include <cstring>

#include "game/gamedb.h"
#include "met/netdodownloadscreen.h"
#include "met/netrepostoolscreen.h"
#include "met/netsavereposlistscreen.h"
#include "met/uploadtoollist.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/listat.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kListComponent[] = "list";
constexpr char kCursorComponent[] = "cursor";
constexpr char kSortDateButton[] = "sort_date";
constexpr char kSortBandButton[] = "sort_band";
constexpr char kSortTitleButton[] = "sort_title";
constexpr char kSortReposButton[] = "sort_repos";
constexpr char kDownloadScreen[] = "fn_do_download_tool";
constexpr char kPreviewRemixScreen[] = "fn_do_download_preview_remix";
constexpr char kPreviewGameScreen[] = "fn_do_download_preview_game";
constexpr char kReposToolScreen[] = "fn_repos_tool";
constexpr char kSaveReposScreen[] = "fn_save_repos_tool";
constexpr char kNoText[] = "";

constexpr int kFirstItem = 0;

// Give each listed item a file of the repository a non-zero mark. Each test clears the mark
// first, so an empty repository leaves the mark as it was.
inline void MarkInRepos(UploadInfo &info, std::list<NetRepoRemix> &remixes) {
    for (unsigned int j = 0; j < remixes.size(); ++j) {
        info.mInRepos = 0;
        if (std::strcmp(info.mName.c_str(), ListAt(remixes, static_cast<int>(j))->mFile.c_str()) ==
            0) {
            info.mInRepos = 1;
            break;
        }
    }
}

} // namespace

void NetUploadToolScreen::SetFiles(std::list<String> &files, std::list<NetRepoRemix> &remixes) {
    mFiles.clear();
    for (unsigned int i = 0; i < files.size(); ++i) {
        UploadInfo info;
        info.mName = *ListAt(files, static_cast<int>(i));
        MarkInRepos(info, remixes);
        mFiles.push_back(info);
    }
}

void NetUploadToolScreen::MarkRemixes(std::list<NetRepoRemix> &remixes) {
    for (unsigned int i = 0; i < mFiles.size(); ++i) {
        MarkInRepos(*ListAt(mFiles, static_cast<int>(i)), remixes);
    }
}

void NetUploadToolScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    TheGameDb->SetCommunity(GameDb::kCommunityOnline);
    static_cast<UploadToolList *>(TheUI.FindComponent(mFocusPanel->mName, kListComponent, false))
        ->SetFiles(mFiles, mSelection);
}

void NetUploadToolScreen::Exit(UIScreen *pNextScreen, float fTime) {
    mSelection =
        static_cast<UIList *>(TheUI.FindComponent(mFocusPanel->mName, kListComponent, false))
            ->mSelected;
    FreqScreen::Exit(pNextScreen, fTime);
}

void NetUploadToolScreen::RemoveFile(int nIndex) {
    if (static_cast<unsigned int>(nIndex) < mFiles.size()) {
        mFiles.erase(ListAt(mFiles, nIndex));
    }
}

bool NetUploadToolScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void NetUploadToolScreen::ParseNames(
    int nPart, const char *pszA, const char *pszB, char *pszPartA, char *pszPartB) {
    char szDateA[kUploadDateSize] = "";
    char szDateB[kUploadDateSize] = "";
    char szSongA[kUploadPartSize] = "";
    char szSongB[kUploadPartSize] = "";
    char szTitleA[kUploadPartSize] = "";
    char szTitleB[kUploadPartSize] = "";
    SplitUploadName(pszA, szDateA, szSongA, szTitleA);
    SplitUploadName(pszB, szDateB, szSongB, szTitleB);
    // The binary copies each part without its terminator into the zeroed buffer of the caller.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
    if (nPart == kNamePartDate) {
        std::strncpy(pszPartB, szDateB, std::strlen(szDateB));
        std::strncpy(pszPartA, szDateA, std::strlen(szDateA));
    } else if (nPart == kNamePartSong) {
        std::strncpy(pszPartB, szSongB, std::strlen(szSongB));
        std::strncpy(pszPartA, szSongA, std::strlen(szSongA));
    } else if (nPart == kNamePartTitle) {
        std::strncpy(pszPartB, szTitleB, std::strlen(szTitleB));
        std::strncpy(pszPartA, szTitleA, std::strlen(szTitleA));
    }
#pragma GCC diagnostic pop
}

bool NetUploadToolScreen::LessBySong(const UploadInfo &a, const UploadInfo &b) {
    char szA[kUploadPartSize] = "";
    char szB[kUploadPartSize] = "";
    ParseNames(kNamePartSong, a.mName.c_str(), b.mName.c_str(), szA, szB);
    return std::strcmp(szA, szB) < 0;
}

bool NetUploadToolScreen::LessByTitle(const UploadInfo &a, const UploadInfo &b) {
    char szA[kUploadPartSize] = "";
    char szB[kUploadPartSize] = "";
    ParseNames(kNamePartTitle, a.mName.c_str(), b.mName.c_str(), szA, szB);
    return std::strcmp(szA, szB) < 0;
}

bool NetUploadToolScreen::LessByDate(const UploadInfo &a, const UploadInfo &b) {
    char szA[kUploadDateSize] = "";
    char szB[kUploadDateSize] = "";
    ParseNames(kNamePartDate, a.mName.c_str(), b.mName.c_str(), szA, szB);
    return std::strcmp(szA, szB) < 0;
}

bool NetUploadToolScreen::InReposFirst(const UploadInfo &a, const UploadInfo &b) {
    return static_cast<unsigned int>(b.mInRepos) < static_cast<unsigned int>(a.mInRepos);
}

bool NetUploadToolScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }
    auto *pList = static_cast<UploadToolList *>(
        TheUI.FindComponent(mFocusPanel->mName, kListComponent, false));
    const char *pszButton = pMsg->mComponent->mName;
    if (std::strcmp(pszButton, kCursorComponent) == 0) {
        const UploadInfo &info = *ListAt(mFiles, pList->mSelected);
        if (info.mInRepos == 0) {
            auto *pDownload =
                dynamic_cast<NetDoDownloadScreen *>(TheUI.FindScreen(kDownloadScreen, false));
            const int nSelected = pList->mSelected;
            pDownload->mFile = info.mName.c_str();
            pDownload->mRemixName = kNoText;
            pDownload->mUploadIndex = nSelected;
            TheUI.GotoScreen(pDownload);
        }
        return UIScreen::HandleSelect(pMsg);
    }
    if (std::strcmp(pszButton, kSortDateButton) == 0) {
        mFiles.sort(LessByDate);
    } else if (std::strcmp(pszButton, kSortBandButton) == 0) {
        mFiles.sort(LessBySong);
    } else if (std::strcmp(pszButton, kSortTitleButton) == 0) {
        mFiles.sort(LessByTitle);
    } else if (std::strcmp(pszButton, kSortReposButton) == 0) {
        mFiles.sort(InReposFirst);
    }
    // Yes, the binary refreshes the list after any other button as well.
    auto *pSorted = static_cast<UploadToolList *>(
        TheUI.FindComponent(mFocusPanel->mName, kListComponent, false));
    pSorted->SetFiles(mFiles, kFirstItem);
    mFocusPanel->SetFocus(pSorted, kPadNone);
    return UIScreen::HandleSelect(pMsg);
}

bool NetUploadToolScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return UIScreen::HandleJoypad(pMsg);
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    auto *pList = static_cast<UploadToolList *>(
        TheUI.FindComponent(mFocusPanel->mName, kListComponent, false));
    const int nSelected = pList->mSelected;
    if (mFocusPanel->mFocus != pList) {
        if (pMsg->mButton != kPadDDown) {
            return UIScreen::HandleJoypad(pMsg);
        }
        mFocusPanel->SetFocus(pList, kPadNone);
        pList->SetCursorSelected(true);
        return true;
    }
    if (static_cast<unsigned int>(nSelected) >= mFiles.size()) {
        return UIScreen::HandleJoypad(pMsg);
    }
    const UploadInfo info = *ListAt(mFiles, nSelected);
    if (pMsg->mButton == kPadSquare) {
        TheNetLobby->DeleteFile(this, info.mName.c_str());
        mFiles.erase(ListAt(mFiles, nSelected));
        pList->SetFiles(mFiles, nSelected);
        auto *pRepos =
            dynamic_cast<NetReposToolScreen *>(TheUI.FindScreen(kReposToolScreen, false));
        if (pRepos->RemoveRemix(info)) {
            auto *pSave =
                dynamic_cast<NetSaveReposListScreen *>(TheUI.FindScreen(kSaveReposScreen, false));
            pSave->mRemixes = pRepos->mRemixes;
            TheUI.GotoScreen(pSave);
        }
    } else if (pMsg->mButton == kPadSelect) {
        // Yes, the binary discards the cast.
        (void)dynamic_cast<NetReposToolScreen *>(TheUI.FindScreen(kReposToolScreen, false));
        TheUI.GotoScreen(kReposToolScreen);
    } else if (pMsg->mButton == kPadR1 || pMsg->mButton == kPadR2) {
        const char *pszScreen = pMsg->mButton == kPadR1 ? kPreviewRemixScreen : kPreviewGameScreen;
        auto *pPreview = dynamic_cast<NetDoDownloadScreen *>(TheUI.FindScreen(pszScreen, false));
        const int nPreviewed = pList->mSelected;
        pPreview->mFile = info.mName.c_str();
        pPreview->mRemixName = kNoText;
        pPreview->mUploadIndex = nPreviewed;
        TheUI.GotoScreen(pPreview);
    } else if (pMsg->mButton == kPadL1) {
        if (nSelected >= pList->mRowCount) {
            pList->SetSelected(nSelected - pList->mRowCount);
        }
    } else if (pMsg->mButton == kPadL2) {
        if (pList->mItemCount - pList->mRowCount >= nSelected) {
            pList->SetSelected(nSelected + pList->mRowCount);
        }
    }
    return UIScreen::HandleJoypad(pMsg);
}

#endif

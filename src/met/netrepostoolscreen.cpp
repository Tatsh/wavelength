#include "met/netrepostoolscreen.h"

#ifdef VIDEO_STANDARD_PAL

#include <cstring>

#include "met/netsavereposlistscreen.h"
#include "met/netuploadtoolscreen.h"
#include "met/uploadnotescreen.h"
#include "os/joypad.h"
#include "os/listat.h"
#include "ui/uimanager.h"

namespace {

constexpr char kListComponent[] = "list";
constexpr char kUploadToolScreen[] = "fn_upload_tool";
constexpr char kSaveReposScreen[] = "fn_save_repos_tool";
constexpr char kNoteScreen[] = "fn_upload_note_tool";
constexpr char kNoText[] = "";

constexpr int kFirstItem = 0;

} // namespace

bool NetReposToolScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void NetReposToolScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mList = static_cast<ReposToolList *>(
        TheUI.FindComponent(mFocusPanel->mName, kListComponent, false));
    mList->SetRemixes(mRemixes);
    if (mSelection == kNoSelection) {
        mList->SetSelected(kFirstItem);
        return;
    }
    ListAt(mRemixes, mSelection)->mNote = mNote;
    mList->SetRemixes(mRemixes);
    mList->SetSelected(mSelection);
    mNote = kNoText;
    mSelection = kNoSelection;
}

void NetReposToolScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    if (std::strcmp(pNextScreen->mName, kNoteScreen) != 0) {
        mSelection = kNoSelection;
        return;
    }
    mSelection = mList->mSelected;
    auto *pNoteScreen = dynamic_cast<UploadNoteScreen *>(TheUI.FindScreen(kNoteScreen, false));
    const NetRepoRemix &remix = *ListAt(mRemixes, mSelection);
    pNoteScreen->mKeyboardText = remix.mNote.c_str();
    mNote = remix.mNote;
}

bool NetReposToolScreen::RemoveRemix(const UploadInfo &info) {
    auto it = mRemixes.begin();
    for (unsigned int i = 0; i < mRemixes.size(); ++i, ++it) {
        if (std::strcmp(it->mFile.c_str(), info.mName.c_str()) == 0) {
            mRemixes.erase(it);
            return true;
        }
    }
    return false;
}

void NetReposToolScreen::AddRemix(const NetRepoRemix &remix) {
    mRemixes.push_back(remix);
}

bool NetReposToolScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return UIScreen::HandleJoypad(pMsg);
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    const int nSelected = mList->mSelected;
    if (static_cast<unsigned int>(nSelected) >= mRemixes.size()) {
        return UIScreen::HandleJoypad(pMsg);
    }
    switch (pMsg->mButton) {
    case kPadStart: {
        dynamic_cast<NetUploadToolScreen *>(TheUI.FindScreen(kUploadToolScreen, false))
            ->MarkRemixes(mRemixes);
        auto *pSave =
            dynamic_cast<NetSaveReposListScreen *>(TheUI.FindScreen(kSaveReposScreen, false));
        pSave->mRemixes = mRemixes;
        TheUI.GotoScreen(pSave);
        break;
    }
    case kPadSquare:
        mRemixes.erase(ListAt(mRemixes, nSelected));
        mList->SetRemixes(mRemixes);
        break;
    case kPadCircle:
        TheUI.GotoScreen(kNoteScreen);
        break;
    case kPadL1:
        if (nSelected != 0) {
            const NetRepoRemix remix = *ListAt(mRemixes, nSelected);
            auto next = mRemixes.erase(ListAt(mRemixes, nSelected));
            if (next != mRemixes.begin()) {
                --next;
            }
            mRemixes.insert(next, remix);
            mList->SetRemixes(mRemixes);
            mList->SetSelected(nSelected - 1);
        }
        break;
    case kPadL2:
        if (static_cast<unsigned int>(nSelected) < mRemixes.size() - 1) {
            const NetRepoRemix remix = *ListAt(mRemixes, nSelected);
            auto next = mRemixes.erase(ListAt(mRemixes, nSelected));
            mRemixes.insert(++next, remix);
            mList->SetRemixes(mRemixes);
            mList->SetSelected(nSelected + 1);
        }
        break;
    default:
        break;
    }
    return UIScreen::HandleJoypad(pMsg);
}

#endif

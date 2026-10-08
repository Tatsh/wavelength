#include "met/selremixdownloadscreen.h"

#include "met/netdodownloadscreen.h"
#include "met/remixdownloadlist.h"
#include "os/joypad.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kDownloadPanel[] = "fn_download";
constexpr char kListComponent[] = "list";
constexpr char kDoDownloadScreen[] = "fn_do_download";

} // namespace

void SelRemixDownloadScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    auto *pList = static_cast<RemixDownloadList *>(
        TheUI.FindComponent(mFocusPanel->mName, kListComponent, false));
    pList->SetRemixes(mRemixes);
    pList->SetSelected(0);
    mShowNote = 1;
    pList->SetNoteShowing(1);
}

bool SelRemixDownloadScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool SelRemixDownloadScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        auto *pList =
            static_cast<UIList *>(TheUI.FindComponent(kDownloadPanel, kListComponent, false));
        auto remix = mRemixes.begin();
        for (int i = 0; remix != mRemixes.end() && i != pList->mSelected; ++i) {
            ++remix;
        }
        auto *pScreen =
            dynamic_cast<NetDoDownloadScreen *>(TheUI.FindScreen(kDoDownloadScreen, false));
        pScreen->mFile = remix->mFile.c_str();
        pScreen->mRemixName = remix->mInfo.mName;
        TheUI.GotoScreen(pScreen);
    }
    return UIScreen::HandleSelect(pMsg);
}

bool SelRemixDownloadScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mButton == kPadCircle && pMsg->mPressed != 0) {
        auto *pList = static_cast<RemixDownloadList *>(
            TheUI.FindComponent(mFocusPanel->mName, kListComponent, false));
        mShowNote ^= 1;
        pList->SetNoteShowing(mShowNote);
    }
    return UIScreen::HandleJoypad(pMsg);
}

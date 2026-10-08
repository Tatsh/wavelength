#include "met/remixcopydelscreen.h"

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "met/deleteremixscreen.h"
#include "met/errorscreen.h"
#include "met/nameerrorscreen.h"
#include "met/saveremixscreen.h"
#include "os/joypad.h"
#include "synth/fxmidi.h"
#include "ui/uilist.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanel[] = "s_r_load";
constexpr char kListComponent[] = "list";

} // namespace

RemixCopyDelScreen::RemixCopyDelScreen(DataArray *pData) : RemixSelectScreen(pData) {
}

bool RemixCopyDelScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return RemixSelectScreen::DispatchPriv(pMsg);
}

bool RemixCopyDelScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    static_cast<UIList *>(TheUI.FindComponent(kPanel, kListComponent, false))
        ->SetCursorSelected(true);
    return UIScreen::HandleSelect(pMsg);
}

bool RemixCopyDelScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return UIScreen::HandleJoypad(pMsg);
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    UIList *pList = static_cast<UIList *>(TheUI.FindComponent(kPanel, kListComponent, false));
    RemixInfo info = mRemixes[pList->mSelected];
    if (pMsg->mButton == kPadCircle) {
        const int nOtherSlot = mSlot == 0;
        TheGameDb->SetRemix(&info);
        TheGameDb->SetSong(info.mSong);
        TheGameDb->SetLoadRemix(true);
        ErrorScreen *pLoad =
            dynamic_cast<ErrorScreen *>(TheUI.FindScreen("load_remix_copy", false));
        pLoad->SetSlot(mSlot);
        SaveRemixScreen *pSave =
            dynamic_cast<SaveRemixScreen *>(TheUI.FindScreen("save_remix_copy", false));
        pSave->SetSlot(nOtherSlot);
        pSave->mRemixName = info.mName;
        pSave->mOverwriteStatus = 0;
        if (info.mReadOnly == 0) {
            TheUI.GotoScreen("copy_read_only_check");
        } else {
            TheUI.GotoScreen(pLoad);
        }
    } else if (pMsg->mButton == kPadSquare) {
        FxMidi::PlaySquare();
        DeleteRemixScreen *pDelete =
            dynamic_cast<DeleteRemixScreen *>(TheUI.FindScreen("del_remix", false));
        pDelete->mRemixName = info.mName;
        pDelete->SetSlot(mSlot);
        pDelete->SetStartScreen("mem_remix");
        if (mRemixes.size() == 1) {
            pDelete->SetDoneScreen("o_rf_mem");
        } else {
            pDelete->SetDoneScreen("load_remix_list_mc");
        }
        NameErrorScreen *pCheck =
            dynamic_cast<NameErrorScreen *>(TheUI.FindScreen("del_remix_check", false));
        pCheck->SetSlot(mSlot);
        pCheck->mSaveName = info.mName;
        TheUI.GotoScreen(pCheck);
    }
    return UIScreen::HandleJoypad(pMsg);
}

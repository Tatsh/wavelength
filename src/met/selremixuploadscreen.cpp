#include "met/selremixuploadscreen.h"

#include "game/gamedb.h"
#include "met/loadremixscreen.h"
#include "os/joypad.h"
#include "ui/uilist.h"
#include "ui/uimanager.h"

namespace {

constexpr char kUploadPanel[] = "fn_upload";
constexpr char kListComponent[] = "list";
constexpr char kLoadRemixScreen[] = "load_remix";
constexpr char kUploadScreen[] = "fn_upload";
constexpr char kNoteScreen[] = "fn_upload_note";

} // namespace

bool SelRemixUploadScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool SelRemixUploadScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        auto *pList =
            static_cast<UIList *>(TheUI.FindComponent(kUploadPanel, kListComponent, false));
        RemixInfo info = mRemixes[pList->mSelected];
        TheGameDb->SetRemix(&info);
        TheGameDb->SetSong(info.mSong);
        auto *pLoad = dynamic_cast<LoadRemixScreen *>(TheUI.FindScreen(kLoadRemixScreen, false));
        pLoad->SetStartScreen(kUploadScreen);
        pLoad->SetDoneScreen(kNoteScreen);
        TheUI.GotoScreen(pLoad);
    }
    return UIScreen::HandleSelect(pMsg);
}

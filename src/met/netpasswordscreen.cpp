#include "met/netpasswordscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uicomponent.h"
#include "ui/uipanel.h"

namespace {

constexpr char kSaveButton[] = "save";
constexpr char kSavePasswordToken[] = "save_password";
constexpr char kDontSavePasswordToken[] = "dont_save_password";
constexpr char kNoPassword[] = "";

constexpr int kLocalPlayer = 0;

} // namespace

NetPasswordScreen::NetPasswordScreen(DataArray *pData) : FreqScreen(pData), mSavePassword(1) {
}

inline void NetPasswordScreen::LabelSaveChoice(UIComponent *pComponent) {
    if (mSavePassword != 0) {
        pComponent->SetText(TheLocale.Localize(kSavePasswordToken, true));
    } else {
        pComponent->SetText(TheLocale.Localize(kDontSavePasswordToken, true));
    }
}

void NetPasswordScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    UIComponent *pSave = mFocusPanel->FindComponent(kSaveButton, false);
    mSavePasswordInitial =
        std::strcmp(TheGameDb->GetProfile(kLocalPlayer)->mPassword.c_str(), kNoPassword) != 0;
    mSavePassword = mSavePasswordInitial;
    LabelSaveChoice(pSave);
}

bool NetPasswordScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectStartMsgType) {
        return HandleSaveToggle(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetPasswordScreen::HandleSaveToggle(UIComponentSelectStartMsg *pMsg) {
    if (std::strcmp(pMsg->GetComponent()->GetName(), kSaveButton) == 0 &&
        (pMsg->GetButton() == kPadDLeft || pMsg->GetButton() == kPadDRight)) {
        mSavePassword ^= 1;
        LabelSaveChoice(pMsg->GetComponent());
    }
    return HandleSelectStart(pMsg);
}

#include "met/netserverselscreen.h"

#include <cstring>

#include "met/netserverlogin.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kServerPanel[] = "fn_sel_server";
constexpr char kServerLoginScreen[] = "net_server_login";
constexpr char kNoText[] = "";

} // namespace

NetServerSelScreen::NetServerSelScreen(DataArray *pData) : FreqScreen(pData) {
}

void NetServerSelScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    UIPanel *pPanel = TheUI.FindPanel(kServerPanel, false);
    auto location = mLocations.begin();
    for (const auto &entry : pPanel->mComponents) {
        UIComponent *pButton = entry.second;
        if (location != mLocations.end()) {
            pButton->SetState(UIComponent::kStateNormal, false);
            pButton->SetShowing(true);
            pButton->SetText(location->mName.c_str());
            ++location;
        } else {
            pButton->SetState(UIComponent::kStateDisabled, false);
            pButton->SetShowing(false);
            pButton->SetText(kNoText);
        }
    }
    if (!mLocations.empty()) {
        pPanel->mComponents.begin()->second->SetState(UIComponent::kStateSelected, false);
    }
    FreqScreen::Enter(pPrevScreen, fTime);
}

bool NetServerSelScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetServerSelScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        const char *pszLocation = pMsg->mComponent->Text();
        auto location = mLocations.begin();
        while (location != mLocations.end() &&
               std::strcmp(pszLocation, location->mName.c_str()) != 0) {
            ++location;
        }
        NetServerLogin *pLogin =
            static_cast<NetServerLogin *>(TheUI.FindScreen(kServerLoginScreen, false));
        // Yes, the binary does not check that a location of the name was found.
        pLogin->mLocation = location->mId;
        TheUI.GotoScreen(pLogin);
    }
    return UIScreen::HandleSelect(pMsg);
}

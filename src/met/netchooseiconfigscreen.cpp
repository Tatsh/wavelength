#include "met/netchooseiconfigscreen.h"

#include <cstring>
#include <iterator>

#include "met/netinetconnect.h"
#include "os/joypad.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "rnd/view.h"
#include "ui/uibutton.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kConfigPanel[] = "fn_config";
constexpr char kConfigButtonFormat[] = "config%d";
constexpr char kConfigView[] = "fn_config.view";
constexpr char kFirstConfigButton[] = "config1";
constexpr char kCreateButton[] = "create";
constexpr char kDescriptionText[] = "fn_config_data_02.txt";
constexpr char kCreateConfigScreen[] = "create_config_check";
constexpr char kConnectScreen[] = "net_connect_internet";

// The screen has buttons for this many configurations.
constexpr int kConfigButtons = 7;

} // namespace

NetChooseIConfigScreen::NetChooseIConfigScreen(DataArray *pData) : FreqScreen(pData), mSelected(0) {
}

void NetChooseIConfigScreen::SetConfigs(const std::list<InetConfig> &configs) {
    mConfigs = configs;
    mSelected = 0;
}

void NetChooseIConfigScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    const int nCount = static_cast<int>(mConfigs.size());
    for (int i = 0; i < kConfigButtons; ++i) {
        auto *pButton = dynamic_cast<UIButton *>(
            TheUI.FindComponent(kConfigPanel, FormatString(kConfigButtonFormat, i + 1), false));
        if (i < nCount) {
            pButton->SetText(std::next(mConfigs.begin(), i)->mName.c_str());
            pButton->SetShowing(true);
        } else {
            pButton->SetState(UIComponent::kStateDisabled, false);
            pButton->SetShowing(false);
        }
    }

    auto *pView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kConfigView));
    const int nFramesPerButton = static_cast<int>(pView->UnfilterFrame(pView->FilteredFrameEnd()) /
                                                  static_cast<float>(kConfigButtons));
    const float flAllButtons = static_cast<float>(nFramesPerButton * kConfigButtons);
    UIButton *pFocus;
    if (nCount != 0) {
        mSelected = 0;
        pFocus =
            dynamic_cast<UIButton *>(TheUI.FindComponent(kConfigPanel, kFirstConfigButton, false));
        pView->SetFrame(flAllButtons - static_cast<float>(nCount * nFramesPerButton));
        ShowDescription(true);
    } else {
        pView->SetFrame(flAllButtons);
        ShowDescription(false);
        pFocus = dynamic_cast<UIButton *>(TheUI.FindComponent(kConfigPanel, kCreateButton, false));
    }
    TheUI.FindPanel(kConfigPanel, false)->SetFocus(pFocus, kPadNone);
}

void NetChooseIConfigScreen::ShowDescription(bool bShow) {
    auto *pText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(kDescriptionText));
    pText->SetShowing(bShow);
    if (bShow) {
        pText->SetText(std::next(mConfigs.begin(), mSelected)->mDescription.c_str());
    }
}

bool NetChooseIConfigScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetChooseIConfigScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        if (std::strcmp(kCreateButton, pMsg->mComponent->mName) == 0) {
            TheUI.GotoScreen(kCreateConfigScreen);
        } else {
            UIScreen *pConnect = TheUI.FindScreen(kConnectScreen, false);
            static_cast<NetInetConnect *>(pConnect)->mConfig =
                std::next(mConfigs.begin(), mSelected)->mId;
            TheUI.GotoScreen(pConnect);
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

bool NetChooseIConfigScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    if (pMsg->mComponent == nullptr || pMsg->mPanel == nullptr ||
        std::strcmp(pMsg->mPanel->mName, kConfigPanel) != 0) {
        return false;
    }
    if (std::strcmp(pMsg->mComponent->mName, kCreateButton) == 0) {
        ShowDescription(false);
    } else if (!mConfigs.empty()) {
        const char *pszName = pMsg->mComponent->mName;
        mSelected = pszName[std::strlen(pszName) - 1] - '1';
        ShowDescription(true);
    }
    return false;
}

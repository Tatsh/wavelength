#include "ui/uicomponent.h"

#include "msg/message.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kEmptyText[] = "";

} // namespace

int UIComponent::sNumFlashes = 2;
float UIComponent::sSelectedMs = 50.0f;
float UIComponent::sNormalMs = 40.0f;

void UIComponent::Init(DataArray *pConfig) {
    DataArray *pSelect = pConfig->FindArray("button_select", false);
    if (pSelect == nullptr) {
        return;
    }
    pSelect->FindInt("num_flashes", &sNumFlashes, false);
    pSelect->FindFloat("frames_selected", &sSelectedMs, false);
    pSelect->FindFloat("frames_normal", &sNormalMs, false);
}

bool UIComponent::Dispatch(Message *pMsg) {
    const bool bNavigation = TheUI.IsNavigationMsg(pMsg);
    UIPanel *pPanel = TheUI.FocusPanel();
    if (!bNavigation && pPanel->Dispatch(pMsg)) {
        return true;
    }
    return DispatchPriv(pMsg);
}

bool UIComponent::DispatchPriv(Message *pMsg) {
    (void)pMsg->Type(); // Yes, the binary discards the type.
    return false;
}

const char *UIComponent::Text() const {
    return kEmptyText;
}

void UIComponent::SetText([[maybe_unused]] const char *pszText) {
}

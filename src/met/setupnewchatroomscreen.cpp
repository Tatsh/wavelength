#include "met/setupnewchatroomscreen.h"

#include "met/netnewchatroomscreen.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"
#include "ui/uitextentry.h"

namespace {

constexpr char kNameEntry[] = "name";
constexpr char kCreateLobbyScreen[] = "net_create_lobby";

constexpr float kNameMaxWidth = 173.0f;

} // namespace

void SetupNewChatroomScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    SetupNameScreen::Enter(pPrevScreen, fTime);
    mEntry =
        dynamic_cast<UITextEntry *>(TheUI.FindComponent(mFocusPanel->GetName(), kNameEntry, false));
    mEntry->SetMaxEntryWidth(kNameMaxWidth);
}

void SetupNewChatroomScreen::Submit() {
    NetNewChatroomScreen *pCreate =
        dynamic_cast<NetNewChatroomScreen *>(TheUI.FindScreen(kCreateLobbyScreen, false));
    pCreate->SetChatroomName(mEntry->Text());
    TheUI.GotoScreen(pCreate);
}

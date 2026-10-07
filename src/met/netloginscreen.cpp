#include "met/netloginscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/keyboardpanel.h"
#include "met/keyboardrequest.h"
#include "met/netserverlogin.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"
#include "ui/uitextentry.h"

namespace {

constexpr char kLoginPanel[] = "fn_login";
constexpr char kNameEntry[] = "name";
constexpr char kPasswordEntry[] = "password";
constexpr char kPasswordPrefix[] = "password_prefix";
constexpr char kCreateButton[] = "create";
constexpr char kSaveButton[] = "save";
constexpr char kKeyboardPanel[] = "keyboard";
constexpr char kKeyboardScreen[] = "kb_screen";
constexpr char kServerLoginScreen[] = "net_server_login";
constexpr char kNoPassword[] = "";

constexpr int kLocalPlayer = 0;
constexpr int kNameLocked = 1;
constexpr int kMaxPasswordChars = 25;

// The keyboard request of the password.
constexpr int kKeyboardPad = 0;
constexpr int kNoFunctionKeys = 0;
constexpr int kNoInvalidChars = 0;
constexpr int kHidePassword = 1;
constexpr int kSingleLine = 0;

} // namespace

NetLoginScreen::NetLoginScreen(DataArray *pData) : NetPasswordScreen(pData) {
}

void NetLoginScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    NetPasswordScreen::Enter(pPrevScreen, fTime);
    UIPanel *pPanel = mFocusPanel;
    pPanel->FindComponent(kNameEntry, false)
        ->SetText(TheGameDb->GetProfile(kLocalPlayer)->mName.c_str());

    UITextEntry *pPassword =
        static_cast<UITextEntry *>(pPanel->FindComponent(kPasswordEntry, false));
    pPassword->mMaxNumChars = kMaxPasswordChars;
    if (std::strcmp(mTypedPassword.c_str(), kNoPassword) != 0) {
        pPassword->SetText(mTypedPassword.c_str());
        mTypedPassword = kNoPassword;
    } else {
        pPassword->SetText(TheGameDb->GetProfile(kLocalPlayer)->mPassword.c_str());
    }

    if (TheGameDb->GetProfile(kLocalPlayer)->mNameLocked == kNameLocked) {
        pPanel->FindComponent(kCreateButton, false)->SetState(UIComponent::kStateDisabled, false);
        pPanel->SetFocus(pPanel->FindComponent(kPasswordEntry, false), kPadNone);
        pPassword->SetEditing(true);
        pPassword->SetState(UIComponent::kStateSelected, true);
    } else {
        pPanel->SetFocus(pPanel->FindComponent(kCreateButton, false), kPadNone);
        TheUI.FindComponent(kLoginPanel, kPasswordPrefix, false)
            ->SetState(UIComponent::kStateNormal, true);
        pPassword->SetEditing(false);
        pPassword->SetState(UIComponent::kStateNormal, true);
    }
}

void NetLoginScreen::Login() {
    const String password(mFocusPanel->FindComponent(kPasswordEntry, false)->Text());
    NetServerLogin *pLogin =
        static_cast<NetServerLogin *>(TheUI.FindScreen(kServerLoginScreen, false));
    pLogin->mPassword = password.c_str();
    pLogin->mSaveChanged = mSavePassword != mSavePasswordInitial;
    pLogin->mSavePassword = mSavePassword;
    TheNetLobby->SetProfile(TheGameDb->GetProfile(kLocalPlayer));
    TheUI.GotoScreen(pLogin);
    mTypedPassword = kNoPassword;
}

void NetLoginScreen::OpenKeyboard() {
    UIPanel *pPanel = mFocusPanel;
    KeyboardPanel *pKeyboard =
        dynamic_cast<KeyboardPanel *>(TheUI.FindPanel(kKeyboardPanel, false));
    UITextEntry *pPassword =
        static_cast<UITextEntry *>(pPanel->FindComponent(kPasswordEntry, false));
    const int nMaxWidth = static_cast<int>(pPassword->mMaxEntryWidth);
    const KeyboardRequest request(this,
                                  TheUI.mCurrentScreen,
                                  pPassword->Text(),
                                  kKeyboardPad,
                                  kMaxPasswordChars,
                                  nMaxWidth,
                                  kNoFunctionKeys,
                                  kNoInvalidChars,
                                  kHidePassword,
                                  kSingleLine);
    pKeyboard->SetRequest(request);
    TheUI.GotoScreen(kKeyboardScreen);
}

bool NetLoginScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUITextEntryCompleteMsgType) {
        return HandleTextEntryComplete(static_cast<UITextEntryCompleteMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    return NetPasswordScreen::DispatchPriv(pMsg);
}

bool NetLoginScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }

    UIPanel *pPanel = mFocusPanel;
    if (pPanel != nullptr && pMsg->mPressed != 0) {
        const char *pszFocus = pPanel->mFocus->mName;
        if (pMsg->mButton == kPadCross) {
            if (std::strcmp(pszFocus, kPasswordEntry) == 0) {
                UITextEntry *pPassword =
                    dynamic_cast<UITextEntry *>(mFocusPanel->FindComponent(kPasswordEntry, false));
                if (std::strcmp(pPassword->Text(), kNoPassword) == 0) {
                    OpenKeyboard();
                } else {
                    Login();
                }
            } else if (std::strcmp(pszFocus, kSaveButton) == 0) {
                UIComponent *pCreate = pPanel->FindComponent(kCreateButton, false);
                if (pCreate->GetState() != UIComponent::kStateDisabled) {
                    pPanel->SetFocus(pCreate, kPadNone);
                } else {
                    pPanel->SetFocus(pPanel->FindComponent(kPasswordEntry, false), kPadNone);
                }
                return true;
            }
        } else if (pMsg->mButton == kPadCircle && std::strcmp(pszFocus, kPasswordEntry) == 0) {
            OpenKeyboard();
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool NetLoginScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    UITextEntry *pEntry = dynamic_cast<UITextEntry *>(pMsg->mComponent);
    UITextEntry *pPassword =
        static_cast<UITextEntry *>(TheUI.FindComponent(kLoginPanel, kPasswordEntry, false));
    UIComponent *pPrefix = TheUI.FindComponent(kLoginPanel, kPasswordPrefix, false);
    if (pEntry == pPassword) {
        pPrefix->SetState(UIComponent::kStateSelected, false);
        pPassword->SetEditing(true);
    } else {
        pPrefix->SetState(UIComponent::kStateNormal, false);
        pPassword->SetEditing(false);
    }
    return false;
}

int NetLoginScreen::ReceiveKeyboardText(const char *pszText) {
    mTypedPassword = pszText;
    return 1;
}

bool NetLoginScreen::HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg) {
    if (pMsg->mText.mLength != 0) {
        Login();
    }
    return true;
}

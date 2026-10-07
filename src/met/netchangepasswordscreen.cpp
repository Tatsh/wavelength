#include "met/netchangepasswordscreen.h"

#include <cstring>

#include "met/keyboardpanel.h"
#include "met/keyboardrequest.h"
#include "met/netchangepassword.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"
#include "ui/uitextentry.h"

namespace {

constexpr char kPasswordPanel[] = "fn_pass";
constexpr char kOldEntry[] = "old";
constexpr char kNewEntry[] = "new";
constexpr char kConfirmEntry[] = "confirm";
constexpr char kSaveButton[] = "save";
constexpr char kDoneButton[] = "done";
constexpr char kPrefixFormat[] = "%s_prefix";
constexpr char kKeyboardPanel[] = "keyboard";
constexpr char kKeyboardScreen[] = "kb_screen";
constexpr char kChangePasswordScreen[] = "net_change_password";
constexpr char kMismatchScreen[] = "cant_create_password_dlg_error";
constexpr char kNoText[] = "";

constexpr int kMaxPasswordChars = 25;

// The keyboard requests of the entries.
constexpr int kKeyboardPad = 0;
constexpr int kNoFunctionKeys = 0;
constexpr int kNoInvalidChars = 0;
constexpr int kHidePassword = 1;
constexpr int kSingleLine = 0;

// Fill an entry with a retained text, unless the text is empty.
void RestoreEntry(UITextEntry *pEntry, const String &text) {
    if (std::strcmp(text.c_str(), kNoText) != 0) {
        pEntry->SetText(text.c_str());
    }
}

} // namespace

NetChangePasswordScreen::NetChangePasswordScreen(DataArray *pData) : NetPasswordScreen(pData) {
}

void NetChangePasswordScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    UITextEntry *pOld = dynamic_cast<UITextEntry *>(mFocusPanel->FindComponent(kOldEntry, false));
    RestoreEntry(pOld, mOld);
    pOld->mMaxNumChars = kMaxPasswordChars;

    UITextEntry *pNew = dynamic_cast<UITextEntry *>(mFocusPanel->FindComponent(kNewEntry, false));
    RestoreEntry(pNew, mNew);
    pNew->mMaxNumChars = kMaxPasswordChars;

    UITextEntry *pConfirm =
        dynamic_cast<UITextEntry *>(mFocusPanel->FindComponent(kConfirmEntry, false));
    RestoreEntry(pConfirm, mConfirm);
    pConfirm->mMaxNumChars = kMaxPasswordChars;

    if (std::strcmp(mKeyboardText.c_str(), kNoText) != 0) {
        (void)std::strcmp(mKeyboardEntry.c_str(), kNoText); // Yes, the binary discards this.
        dynamic_cast<UITextEntry *>(
            TheUI.FindComponent(kPasswordPanel, mKeyboardEntry.c_str(), false))
            ->SetText(mKeyboardText.c_str());
    }
    mKeyboardEntry = kNoText;
    mKeyboardText = kNoText;
    mNew = kNoText;
    mOld = kNoText;
    mConfirm = kNoText;

    if (std::strcmp(pPrevScreen->mName, kKeyboardScreen) != 0) {
        mFocusPanel->SetFocus(
            dynamic_cast<UITextEntry *>(mFocusPanel->FindComponent(kOldEntry, false)), kPadNone);
    }
    NetPasswordScreen::Enter(pPrevScreen, fTime);
}

void NetChangePasswordScreen::Submit() {
    const String newPassword(mFocusPanel->FindComponent(kNewEntry, false)->Text());
    const String confirm(mFocusPanel->FindComponent(kConfirmEntry, false)->Text());
    if (newPassword != confirm) {
        TheUI.GotoScreen(kMismatchScreen);
        return;
    }

    const String oldPassword(mFocusPanel->FindComponent(kOldEntry, false)->Text());
    NetChangePassword *pChange =
        static_cast<NetChangePassword *>(TheUI.FindScreen(kChangePasswordScreen, false));
    pChange->mOldPassword = oldPassword.c_str();
    pChange->mNewPassword = newPassword.c_str();
    // Yes, the binary reports a change whenever either choice saves the password.
    pChange->mSaveChanged = (mSavePasswordInitial != 0 || mSavePassword != 0);
    pChange->mSavePassword = mSavePassword;
    TheUI.GotoScreen(pChange);
    mNew = kNoText;
}

void NetChangePasswordScreen::OpenKeyboard(const char *pszEntry) {
    mKeyboardEntry = pszEntry;
    mOld = TheUI.FindComponent(kPasswordPanel, kOldEntry, false)->Text();
    mNew = TheUI.FindComponent(kPasswordPanel, kNewEntry, false)->Text();
    mConfirm = TheUI.FindComponent(kPasswordPanel, kConfirmEntry, false)->Text();
    KeyboardPanel *pKeyboard =
        dynamic_cast<KeyboardPanel *>(TheUI.FindPanel(kKeyboardPanel, false));
    const int nMaxWidth =
        static_cast<int>(dynamic_cast<UITextEntry *>(mFocusPanel->mFocus)->mMaxEntryWidth);

    const String *pText = nullptr;
    if (std::strcmp(pszEntry, kOldEntry) == 0) {
        pText = &mOld;
    } else if (std::strcmp(pszEntry, kNewEntry) == 0) {
        pText = &mNew;
    } else if (std::strcmp(pszEntry, kConfirmEntry) == 0) {
        pText = &mConfirm;
    }
    if (pText != nullptr) {
        const KeyboardRequest request(this,
                                      TheUI.mCurrentScreen,
                                      pText->c_str(),
                                      kKeyboardPad,
                                      kMaxPasswordChars,
                                      nMaxWidth,
                                      kNoFunctionKeys,
                                      kNoInvalidChars,
                                      kHidePassword,
                                      kSingleLine);
        pKeyboard->SetRequest(request);
    }
    TheUI.GotoScreen(kKeyboardScreen);
}

bool NetChangePasswordScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUITextEntryCompleteMsgType) {
        return HandleTextEntryComplete(static_cast<UITextEntryCompleteMsg *>(pMsg));
    }
    return NetPasswordScreen::DispatchPriv(pMsg);
}

bool NetChangePasswordScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }

    UIPanel *pPanel = mFocusPanel;
    if (pPanel == nullptr || pMsg->mPressed == 0) {
        return FreqScreen::HandleJoypad(pMsg);
    }

    const char *pszFocus = pPanel->mFocus->mName;
    if (pMsg->mButton == kPadCircle) {
        if (std::strcmp(pszFocus, kOldEntry) == 0 || std::strcmp(pszFocus, kNewEntry) == 0 ||
            std::strcmp(pszFocus, kConfirmEntry) == 0) {
            OpenKeyboard(pszFocus);
        }
    } else if (pMsg->mButton == kPadCross) {
        // The cross button opens the keyboard on an empty entry and otherwise moves on.
        const char *pszNext = nullptr;
        bool bEntry = true;
        if (std::strcmp(pszFocus, kOldEntry) == 0) {
            pszNext = kNewEntry;
        } else if (std::strcmp(pszFocus, kNewEntry) == 0) {
            pszNext = kConfirmEntry;
        } else if (std::strcmp(pszFocus, kConfirmEntry) == 0) {
            pszNext = kSaveButton;
        } else if (std::strcmp(pszFocus, kSaveButton) == 0) {
            pszNext = kDoneButton;
            bEntry = false;
        }
        if (pszNext != nullptr) {
            if (bEntry &&
                std::strcmp(dynamic_cast<UITextEntry *>(pPanel->mFocus)->Text(), kNoText) == 0) {
                OpenKeyboard(pszFocus);
                return true;
            }
            pPanel->SetFocus(pPanel->FindComponent(pszNext, false), kPadNone);
            return true;
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool NetChangePasswordScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    UITextEntry *pOld = dynamic_cast<UITextEntry *>(pMsg->mOldComponent);
    UITextEntry *pNew = dynamic_cast<UITextEntry *>(pMsg->mComponent);
    // Yes, the binary does not check that the prefixes were found.
    if (pOld != nullptr) {
        TheUI.FindComponent(kPasswordPanel, FormatString(kPrefixFormat, pOld->mName), false)
            ->SetState(UIComponent::kStateNormal, false);
        pOld->SetEditing(false);
    }
    if (pNew != nullptr) {
        TheUI.FindComponent(kPasswordPanel, FormatString(kPrefixFormat, pNew->mName), false)
            ->SetState(UIComponent::kStateSelected, false);
        pNew->SetEditing(true);
    }
    return false;
}

bool NetChangePasswordScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (std::strcmp(pMsg->mComponent->mName, kDoneButton) == 0) {
        Submit();
    }
    return UIScreen::HandleSelect(pMsg);
}

bool NetChangePasswordScreen::HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg) {
    const char *pszEntry = pMsg->mEntry->mName;
    const char *pszNext = nullptr;
    if (std::strcmp(pszEntry, kOldEntry) == 0) {
        pszNext = kNewEntry;
    } else if (std::strcmp(pszEntry, kNewEntry) == 0) {
        pszNext = kConfirmEntry;
    } else if (std::strcmp(pszEntry, kConfirmEntry) == 0) {
        pszNext = kSaveButton;
    } else if (std::strcmp(pszEntry, kDoneButton) == 0 && pMsg->mText.mLength != 0) {
        Submit();
    }
    if (pszNext != nullptr) {
        UIPanel *pPanel = mFocusPanel;
        pPanel->SetFocus(pPanel->FindComponent(pszNext, false), kPadNone);
    }
    return false;
}

int NetChangePasswordScreen::ReceiveKeyboardText(const char *pszText) {
    mKeyboardText = pszText;
    return 1;
}

#include "met/netcreateuserscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/keyboardpanel.h"
#include "met/keyboardrequest.h"
#include "met/metagameutil.h"
#include "met/netcreateaccount.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "synth/fxmidi.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"
#include "ui/uitextentry.h"

namespace {

constexpr char kUserPanel[] = "fn_fnew";
constexpr char kNameEntry[] = "name";
constexpr char kPasswordEntry[] = "pass_create";
constexpr char kConfirmEntry[] = "pass_confirm";
constexpr char kSaveButton[] = "save";
constexpr char kLoginButton[] = "login";
constexpr char kPrefixFormat[] = "%s_prefix";
constexpr char kKeyboardPanel[] = "keyboard";
constexpr char kKeyboardScreen[] = "kb_screen";
constexpr char kCreateAccountScreen[] = "net_create_account";
constexpr char kMismatchScreen[] = "cant_confirm_password_dlg_error";
constexpr char kEmptyNameScreen[] = "no_empty_filename_screen";
constexpr char kSpacesScreen[] = "no_lead_trail_spaces_screen";
constexpr char kErrorOkButton[] = "ok";
constexpr char kErrorReturnScreen[] = "net_create_setup";
constexpr char kSavePasswordToken[] = "save_password";
constexpr char kNoText[] = "";

constexpr int kLocalPlayer = 0;
constexpr float kNameMaxWidth = 115.0f;
constexpr int kMaxNameChars = 15;
constexpr int kMaxPasswordChars = 25;

// The keyboard requests of the entries.
constexpr int kKeyboardPad = 0;
constexpr int kNameMaxWidthPixels = 115;
constexpr int kNoFunctionKeys = 0;
constexpr int kCheckInvalidChars = 1;
constexpr int kNoInvalidChars = 0;
constexpr int kShowText = 0;
constexpr int kHidePassword = 1;
constexpr int kSingleLine = 0;

// Show an error screen that leads back to this screen.
void ShowEntryError(const char *pszScreen) {
    UIScreen *pError = TheUI.FindScreen(pszScreen, false);
    pError->ClearTransitions();
    pError->AddTransition(kErrorOkButton, kPadNone, kErrorReturnScreen);
    TheUI.GotoScreen(pError);
}

// Fill an entry with a retained text, unless the text is empty.
void RestoreEntry(UITextEntry *pEntry, const String &text) {
    if (std::strcmp(text.c_str(), kNoText) != 0) {
        pEntry->SetText(text.c_str());
    }
}

} // namespace

NetCreateUserScreen::NetCreateUserScreen(DataArray *pData) : NetPasswordScreen(pData) {
}

void NetCreateUserScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    UITextEntry *pName = dynamic_cast<UITextEntry *>(mFocusPanel->FindComponent(kNameEntry, false));
    if (std::strcmp(mUserName.c_str(), kNoText) == 0) {
        pName->SetText(TheGameDb->GetProfile(kLocalPlayer)->mName.c_str());
    } else {
        pName->SetText(mUserName.c_str());
    }
    pName->mMaxNumChars = kMaxNameChars;
    pName->mMaxEntryWidth = kNameMaxWidth;

    UITextEntry *pPassword =
        dynamic_cast<UITextEntry *>(mFocusPanel->FindComponent(kPasswordEntry, false));
    RestoreEntry(pPassword, mPassword);
    pPassword->mMaxNumChars = kMaxPasswordChars;

    UITextEntry *pConfirm =
        dynamic_cast<UITextEntry *>(mFocusPanel->FindComponent(kConfirmEntry, false));
    RestoreEntry(pConfirm, mConfirm);
    pConfirm->mMaxNumChars = kMaxPasswordChars;

    if (std::strcmp(mKeyboardText.c_str(), kNoText) != 0) {
        (void)std::strcmp(mKeyboardEntry.c_str(), kNoText); // Yes, the binary discards this.
        dynamic_cast<UITextEntry *>(TheUI.FindComponent(kUserPanel, mKeyboardEntry.c_str(), false))
            ->SetText(mKeyboardText.c_str());
        mKeyboardEntry = kNoText;
        mKeyboardText = kNoText;
    }
    mUserName = kNoText;
    mPassword = kNoText;
    mConfirm = kNoText;

    NetPasswordScreen::Enter(pPrevScreen, fTime);
    mFocusPanel->FindComponent(kSaveButton, false)
        ->SetText(TheLocale.Localize(kSavePasswordToken, true));
    mSavePasswordInitial = 1;
    mSavePassword = 1;
}

void NetCreateUserScreen::Submit() {
    const String password(mFocusPanel->FindComponent(kPasswordEntry, false)->Text());
    const String confirm(mFocusPanel->FindComponent(kConfirmEntry, false)->Text());
    if (password != confirm) {
        TheUI.GotoScreen(kMismatchScreen);
        return;
    }

    String name(mFocusPanel->FindComponent(kNameEntry, false)->Text());
    const bool bTrimmed = TrimSpaces(&name);
    if (name.mLength == 0) {
        mKeyboardEntry = kNoText;
        mKeyboardText = kNoText;
        mUserName = kNoText;
        mPassword = kNoText;
        mConfirm = kNoText;
        ShowEntryError(kEmptyNameScreen);
        return;
    }
    if (bTrimmed) {
        mUserName = name.c_str();
        mKeyboardEntry = kNoText;
        mKeyboardText = kNoText;
        mPassword = kNoText;
        mConfirm = kNoText;
        ShowEntryError(kSpacesScreen);
        return;
    }

    TheGameDb->GetProfile(kLocalPlayer)->mName = name.c_str();
    TheNetLobby->SetProfile(TheGameDb->GetProfile(kLocalPlayer));
    NetCreateAccount *pCreate =
        static_cast<NetCreateAccount *>(TheUI.FindScreen(kCreateAccountScreen, false));
    pCreate->mPassword = password.c_str();
    TheUI.GotoScreen(pCreate);
}

void NetCreateUserScreen::OpenKeyboard(const char *pszEntry) {
    mKeyboardEntry = pszEntry;
    mUserName = TheUI.FindComponent(kUserPanel, kNameEntry, false)->Text();
    mPassword = TheUI.FindComponent(kUserPanel, kPasswordEntry, false)->Text();
    mConfirm = TheUI.FindComponent(kUserPanel, kConfirmEntry, false)->Text();
    KeyboardPanel *pKeyboard =
        dynamic_cast<KeyboardPanel *>(TheUI.FindPanel(kKeyboardPanel, false));
    const int nMaxWidth =
        static_cast<int>(dynamic_cast<UITextEntry *>(mFocusPanel->mFocus)->mMaxEntryWidth);

    if (std::strcmp(pszEntry, kNameEntry) == 0) {
        const KeyboardRequest request(this,
                                      TheUI.mCurrentScreen,
                                      mUserName.c_str(),
                                      kKeyboardPad,
                                      kMaxNameChars,
                                      kNameMaxWidthPixels,
                                      kNoFunctionKeys,
                                      kCheckInvalidChars,
                                      kShowText,
                                      kSingleLine);
        pKeyboard->SetRequest(request);
    } else if (std::strcmp(pszEntry, kPasswordEntry) == 0) {
        const KeyboardRequest request(this,
                                      TheUI.mCurrentScreen,
                                      mPassword.c_str(),
                                      kKeyboardPad,
                                      kMaxPasswordChars,
                                      nMaxWidth,
                                      kNoFunctionKeys,
                                      kNoInvalidChars,
                                      kHidePassword,
                                      kSingleLine);
        pKeyboard->SetRequest(request);
    } else if (std::strcmp(pszEntry, kConfirmEntry) == 0) {
        const KeyboardRequest request(this,
                                      TheUI.mCurrentScreen,
                                      mConfirm.c_str(),
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

bool NetCreateUserScreen::DispatchPriv(Message *pMsg) {
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
    if (nType == g_nUITextEntryInvalidMsgType) {
        return HandleTextEntryInvalid(static_cast<UITextEntryInvalidMsg *>(pMsg));
    }
    return NetPasswordScreen::DispatchPriv(pMsg);
}

bool NetCreateUserScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }

    UIPanel *pPanel = mFocusPanel;
    if (pPanel == nullptr || pMsg->mPressed == 0) {
        return FreqScreen::HandleJoypad(pMsg);
    }

    const char *pszFocus = pPanel->mFocus->mName;
    if (pMsg->mButton == kPadCircle) {
        if (std::strcmp(pszFocus, kNameEntry) == 0 || std::strcmp(pszFocus, kPasswordEntry) == 0 ||
            std::strcmp(pszFocus, kConfirmEntry) == 0) {
            OpenKeyboard(pszFocus);
        }
    } else if (pMsg->mButton == kPadCross) {
        // The cross button opens the keyboard on an empty entry and otherwise moves on.
        const char *pszNext = nullptr;
        bool bEntry = true;
        if (std::strcmp(pszFocus, kNameEntry) == 0) {
            pszNext = kPasswordEntry;
        } else if (std::strcmp(pszFocus, kPasswordEntry) == 0) {
            pszNext = kConfirmEntry;
        } else if (std::strcmp(pszFocus, kConfirmEntry) == 0) {
            pszNext = kSaveButton;
        } else if (std::strcmp(pszFocus, kSaveButton) == 0) {
            pszNext = kLoginButton;
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

bool NetCreateUserScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    UITextEntry *pOld = dynamic_cast<UITextEntry *>(pMsg->mOldComponent);
    UITextEntry *pNew = dynamic_cast<UITextEntry *>(pMsg->mComponent);
    if (pMsg->mOldComponent != nullptr) {
        UIComponent *pPrefix = TheUI.FindComponent(
            kUserPanel, FormatString(kPrefixFormat, pMsg->mOldComponent->mName), true);
        if (pPrefix != nullptr) {
            pPrefix->SetState(UIComponent::kStateNormal, false);
        }
    }
    if (pMsg->mComponent != nullptr) {
        UIComponent *pPrefix = TheUI.FindComponent(
            kUserPanel, FormatString(kPrefixFormat, pMsg->mComponent->mName), true);
        if (pPrefix != nullptr) {
            pPrefix->SetState(UIComponent::kStateSelected, false);
        }
    }
    if (pOld != nullptr) {
        pOld->SetEditing(false);
    }
    if (pNew != nullptr) {
        pNew->SetEditing(true);
    }
    return false;
}

bool NetCreateUserScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (std::strcmp(pMsg->mComponent->mName, kLoginButton) == 0) {
        Submit();
    }
    return UIScreen::HandleSelect(pMsg);
}

bool NetCreateUserScreen::HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg) {
    const char *pszEntry = pMsg->mEntry->mName;
    const char *pszNext = nullptr;
    if (std::strcmp(pszEntry, kNameEntry) == 0) {
        pszNext = kPasswordEntry;
    } else if (std::strcmp(pszEntry, kPasswordEntry) == 0) {
        pszNext = kConfirmEntry;
    } else if (std::strcmp(pszEntry, kConfirmEntry) == 0) {
        pszNext = kSaveButton;
    } else if (std::strcmp(pszEntry, kLoginButton) == 0 && pMsg->mText.mLength != 0) {
        Submit();
    }
    if (pszNext != nullptr) {
        UIPanel *pPanel = mFocusPanel;
        pPanel->SetFocus(pPanel->FindComponent(pszNext, false), kPadNone);
    }
    return false;
}

bool NetCreateUserScreen::HandleTextEntryInvalid(UITextEntryInvalidMsg *pMsg) {
    if (std::strcmp(pMsg->mEntry->mName, kNameEntry) == 0) {
        FxMidi::PlayWrong();
    }
    return false;
}

int NetCreateUserScreen::ReceiveKeyboardText(const char *pszText) {
    mKeyboardText = pszText;
    return 1;
}

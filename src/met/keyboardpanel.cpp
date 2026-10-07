#include "met/keyboardpanel.h"

#include <stdio.h>
#include <string.h>

#include "game/gamefx.h"
#include "met/keyboardkey.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/manager.h"
#include "ui/uimanager.h"

namespace {

// The keys the text entry acts on besides the printable characters.
constexpr int kKeyBackspace = 8;
constexpr int kKeyTab = 9;
constexpr int kKeyReturn = 10;
constexpr int kKeyDelete = 311;
constexpr int kKeyLeft = 320;
constexpr int kKeyRight = 321;
constexpr int kKeyF1 = 401;

// The printable characters run from the space for this many codes.
constexpr int kFirstPrintable = ' ';
constexpr int kPrintableCount = 95;

// The names of the function keys, from F1.
const char *const kFunctionKeys[] = {
    "f1",
    "f2",
    "f3",
    "f4",
    "f5",
    "f6",
    "f7",
    "f8",
    "f9",
    "f10",
    "f11",
    "f12",
};
constexpr int kNumFunctionKeys = sizeof(kFunctionKeys) / sizeof(kFunctionKeys[0]);

// The function key buttons are `but_f1` to `but_f12`. The text of the key starts at this letter.
constexpr char kFunctionKeyLetter = 'f';

// The size of the token of a function key's text.
constexpr int kTokenSize = 16;

// The suffixes of the texts of the keys in each shift state.
const char *const kNoSuffix = "";
const char *const kShiftSuffix = "_shift";
const char *const kCapsSuffix = "_caps";

UIButton *FindKey(const char *pszKey) {
    return dynamic_cast<UIButton *>(TheUI.FindComponent("keyboard", pszKey, false));
}

} // namespace

KeyboardPanel::KeyboardPanel(DataArray *pData, const char *pszDir)
    : FreqPanel(pData, pszDir), mCapsButton(nullptr), mLeftShiftButton(nullptr),
      mRightShiftButton(nullptr), mSelectAnim(nullptr) {
}

KeyboardPanel::~KeyboardPanel() {
}

void KeyboardPanel::Enter(bool bForce, float fTime) {
    FreqPanel::Enter(bForce, fTime);
    mCapsButton = FindKey("but_caps");
    mLeftShiftButton = FindKey("but_lshift");
    mRightShiftButton = FindKey("but_rshift");
    mEntry = dynamic_cast<UITextEntry *>(TheUI.FindComponent("keyboard", "01", false));
    mEntry->SetEditing(true);
    mEntry->mPassword = mRequest.mPassword != 0;
    if (mRequest.mInvalidChars != 0) {
        // Yes, the binary looks up `invalid_chars` inside `invalid_chars`.
        mEntry->mInvalidChars = SystemConfig()
                                    ->FindArray("metagame", true)
                                    ->FindArray("invalid_chars", true)
                                    ->FindArray("invalid_chars", true);
    } else {
        mEntry->mInvalidChars = nullptr;
    }
    SetShiftMode(kShiftNone);
    mEntry->mMaxEntryWidth = static_cast<float>(mRequest.mMaxWidth);
    mEntry->mMaxNumChars = mRequest.mMaxChars;
    if (mRequest.mNumLines != 0) {
        mEntry->mScroll = true;
        mEntry->mNumLines = mRequest.mNumLines;
    } else {
        mEntry->mScroll = false;
        mEntry->mNumLines = 1;
    }
    if (mRequest.mText.mLength != 0) {
        mReplaceText = 1;
    }
    mEntry->SetText(mRequest.mText.c_str());
    mSelectAnim = dynamic_cast<Rnd::MatAnim *>(Rnd::TheManager.Find("keyboard_sel.mnm"));
    SetFocus(FindComponent("but_enter", false), kPadNone);
}

void KeyboardPanel::Exit(bool bForce, float fTime) {
    FreqPanel::Exit(bForce, fTime);
    mSelectAnim = nullptr;
    mCapsButton = nullptr;
    mLeftShiftButton = nullptr;
    mRightShiftButton = nullptr;
}

void KeyboardPanel::SetFocus(UIComponent *pComponent, int nButton) {
    GameFx::PlayKeyboardLeftUp();
    UIPanel::SetFocus(pComponent, nButton); // Yes, the binary bypasses FreqPanel::SetFocus().
}

void KeyboardPanel::Poll(float fTime) {
    FreqPanel::Poll(fTime);
    if (mSelectAnim != nullptr) {
        mSelectAnim->SetFrame(fTime);
    }
}

void KeyboardPanel::SetRequest(const KeyboardRequest &request) {
    mRequest = request;
}

bool KeyboardPanel::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const char *pszKey = pMsg->mComponent->mName;
    if (strcmp(pszKey, "but_caps") == 0) {
        SetShiftMode(kShiftCaps);
    } else if (strcmp(pszKey, "but_lshift") == 0 || strcmp(pszKey, "but_rshift") == 0) {
        SetShiftMode(kShiftOnce);
    } else if (strcmp(pszKey, "but_back") == 0) {
        Backspace();
        GameFx::PlayKeyboardBack();
        return true;
    } else if (strcmp(pszKey, "but_larrow") == 0) {
        MoveLeft();
    } else if (strcmp(pszKey, "but_rarrow") == 0) {
        MoveRight();
    } else if (strcmp(pszKey, "but_tab") == 0) {
        TypeTab();
    } else if (strcmp(pszKey, "but_space") == 0) {
        TypeSpace();
    } else if (strcmp(pszKey, "but_del") == 0) {
        Delete();
    } else if (strcmp(pszKey, "but_enter") == 0) {
        Commit();
        GameFx::PlayMenuSelect();
        return true;
    } else {
        const char *pszFunctionKey = strchr(pszKey, kFunctionKeyLetter);
        if (pszFunctionKey != nullptr) {
            TypeFunctionKey(pszFunctionKey);
        } else {
            TypeChar(pMsg->mComponent->Text()[0]);
        }
    }
    GameFx::PlayKeyboardKeyEnter();
    return true;
}

void KeyboardPanel::FlashKey(const char *pszKey) {
    dynamic_cast<KeyboardKey *>(FindComponent(pszKey, false))->Flash();
}

bool KeyboardPanel::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPad != mRequest.mPad) {
        return true;
    }
    if (pMsg->mPressed == 0) {
        return false;
    }
    switch (pMsg->mButton) {
    case kPadTriangle:
        TheUI.GotoScreen(mRequest.mReturnScreen);
        return false;
    case kPadCircle:
        FlashKey("but_enter");
        Commit();
        GameFx::PlayMenuSelect();
        return true;
    case kPadSquare:
        FlashKey("but_space");
        TypeSpace();
        GameFx::PlayKeyboardKeyEnter();
        return true;
    case kPadL1:
        FlashKey("but_larrow");
        MoveLeft();
        GameFx::PlayKeyboardLeftUp();
        return true;
    case kPadR1:
        FlashKey("but_rarrow");
        MoveRight();
        GameFx::PlayKeyboardLeftUp();
        return true;
    case kPadL2:
        FlashKey("but_back");
        Backspace();
        GameFx::PlayKeyboardBack();
        return true;
    case kPadR2:
        SetShiftMode(kShiftOnce);
        GameFx::PlayKeyboardKeyEnter();
        return true;
    default:
        return false;
    }
}

bool KeyboardPanel::HandleKey(KeyboardKeyMsg *pMsg) {
    const int nKey = pMsg->mKey;
    if (static_cast<unsigned>(nKey - kFirstPrintable) < static_cast<unsigned>(kPrintableCount)) {
        TypeChar(static_cast<char>(nKey));
    } else if (nKey == kKeyLeft) {
        MoveLeft();
    } else if (nKey == kKeyRight) {
        MoveRight();
    } else if (nKey == kKeyBackspace) {
        Backspace();
    } else if (nKey == kKeyTab) {
        TypeTab();
    } else if (nKey == kKeyDelete) {
        Delete();
    } else if (nKey == kKeyReturn) {
        Commit();
    } else if (nKey >= kKeyF1 && nKey < kKeyF1 + kNumFunctionKeys) {
        TypeFunctionKey(kFunctionKeys[nKey - kKeyF1]);
    }
    return false;
}

void KeyboardPanel::SetShiftMode(int nMode) {
    const char *pszSuffix;
    if (nMode == kShiftNone) {
        UIStyle *pStyle = TheUI.FindStyle("key_style", false);
        mCapsButton->SetStyle(pStyle);
        mLeftShiftButton->SetStyle(pStyle);
        mRightShiftButton->SetStyle(pStyle);
        if (mShiftMode == kShiftNone) {
            return;
        }
        mShiftMode = kShiftNone;
        pszSuffix = kNoSuffix;
    } else if (nMode == kShiftOnce) {
        if (mShiftMode == kShiftOnce) {
            UIStyle *pStyle = TheUI.FindStyle("key_style", false);
            mLeftShiftButton->SetStyle(pStyle);
            mRightShiftButton->SetStyle(pStyle);
            mShiftMode = kShiftNone;
            pszSuffix = kNoSuffix;
        } else {
            mCapsButton->SetStyle(TheUI.FindStyle("key_style", false));
            UIStyle *pStyle = TheUI.FindStyle("key_high_style", false);
            mLeftShiftButton->SetStyle(pStyle);
            mRightShiftButton->SetStyle(pStyle);
            mShiftMode = kShiftOnce;
            pszSuffix = kShiftSuffix;
        }
    } else if (mShiftMode == kShiftCaps) {
        mCapsButton->SetStyle(TheUI.FindStyle("key_style", false));
        mShiftMode = kShiftNone;
        pszSuffix = kNoSuffix;
    } else {
        mCapsButton->SetStyle(TheUI.FindStyle("key_high_style", false));
        UIStyle *pStyle = TheUI.FindStyle("key_style", false);
        mLeftShiftButton->SetStyle(pStyle);
        mRightShiftButton->SetStyle(pStyle);
        mShiftMode = kShiftCaps;
        pszSuffix = kCapsSuffix;
    }

    for (const auto &entry : mComponents) {
        UIButton *pKey = entry.second != nullptr ? dynamic_cast<UIButton *>(entry.second) : nullptr;
        if (pKey != nullptr && pKey->Text() != nullptr) {
            pKey->SetText(TheLocale.Localize(FormatString("%s%s", pKey->mName, pszSuffix), true));
        }
    }
}

void KeyboardPanel::Backspace() {
    mReplaceText = 0;
    mEntry->HandleKey(kKeyBackspace);
}

void KeyboardPanel::MoveLeft() {
    mReplaceText = 0;
    mEntry->HandleKey(kKeyLeft);
}

void KeyboardPanel::MoveRight() {
    mReplaceText = 0;
    mEntry->HandleKey(kKeyRight);
}

void KeyboardPanel::TypeTab() {
    TypeChar(kKeyTab);
}

void KeyboardPanel::TypeSpace() {
    TypeChar(' ');
}

void KeyboardPanel::Delete() {
    mReplaceText = 0;
    mEntry->HandleKey(kKeyDelete);
}

void KeyboardPanel::Commit() {
    int nReturn = 1;
    if (mRequest.mUser != nullptr) {
        nReturn = mRequest.mUser->ReceiveKeyboardText(mEntry->Text());
    }
    if (mRequest.mReturnScreen == nullptr) {
        DebugWarn(" Keyboard has no screen to transition to!");
        return;
    }
    if (nReturn != 0) {
        TheUI.GotoScreen(mRequest.mReturnScreen);
    }
}

void KeyboardPanel::TypeFunctionKey(const char *pszKey) {
    if (mRequest.mFunctionKeys == 0) {
        TheUI.GotoScreen("kb_fkey_error_screen");
        return;
    }
    char szToken[kTokenSize];
    sprintf(szToken, "fkey_%s", pszKey);
    mReplaceText = 0;
    mEntry->SetText(TheLocale.Localize(szToken, true));
}

void KeyboardPanel::TypeChar(char ch) {
    if (mReplaceText != 0) {
        mReplaceText = 0;
        mEntry->SetText("");
    }
    mEntry->HandleKey(ch);
    if (mShiftMode == kShiftOnce) {
        SetShiftMode(kShiftNone);
    }
}

bool KeyboardPanel::HandleInvalid(UITextEntryInvalidMsg *pMsg) {
    if (strcmp(pMsg->mEntry->mName, "01") != 0) {
        return false;
    }
    if (pMsg->mEndOfField) {
        GameFx::PlayWrong();
        return false;
    }
    if (mShiftMode == kShiftOnce || mShiftMode == kShiftCaps) {
        SetShiftMode(kShiftNone);
    }
    TheUI.GotoScreen("kb_badkey_error_screen");
    return false;
}

bool KeyboardPanel::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nKeyboardKeyMsgType) {
        return HandleKey(static_cast<KeyboardKeyMsg *>(pMsg));
    }
    if (nType == g_nUITextEntryInvalidMsgType) {
        return HandleInvalid(static_cast<UITextEntryInvalidMsg *>(pMsg));
    }
    return FreqPanel::DispatchPriv(pMsg);
}

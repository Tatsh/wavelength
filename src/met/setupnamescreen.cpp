#include "met/setupnamescreen.h"

#include <cstring>

#include "met/keyboardpanel.h"
#include "met/keyboardrequest.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kNameEntry[] = "name";
constexpr char kKeyboardPanel[] = "keyboard";
constexpr char kKeyboardScreen[] = "kb_screen";
constexpr char kNoText[] = "";

// The keyboard request of the entry.
constexpr int kKeyboardPad = 0;
constexpr int kNoFunctionKeys = 0;
constexpr int kNoInvalidChars = 0;
constexpr int kShowText = 0;
constexpr int kSingleLine = 0;

} // namespace

SetupNameScreen::SetupNameScreen(DataArray *pData) : FreqScreen(pData) {
}

void SetupNameScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mEntry =
        dynamic_cast<UITextEntry *>(TheUI.FindComponent(mFocusPanel->mName, kNameEntry, false));
    mEntry->SetText(mText.c_str());
    mText = kNoText;
    mEntry->SetEditing(true);
    mFocusPanel->SetFocus(mEntry, kPadNone);
}

int SetupNameScreen::ReceiveKeyboardText(const char *pszText) {
    mText = pszText;
    return 1;
}

bool SetupNameScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUITextEntryCompleteMsgType) {
        return HandleTextEntryComplete(static_cast<UITextEntryCompleteMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool SetupNameScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (mFocusPanel == nullptr || std::strcmp(mFocusPanel->mFocus->mName, kNameEntry) != 0 ||
        pMsg->mPressed == 0) {
        return FreqScreen::HandleJoypad(pMsg);
    }

    if (pMsg->mButton == kPadCircle) {
        KeyboardPanel *pKeyboard =
            dynamic_cast<KeyboardPanel *>(TheUI.FindPanel(kKeyboardPanel, false));
        UIScreen *pScreen = TheUI.mCurrentScreen;
        const int nMaxWidth = static_cast<int>(mEntry->mMaxEntryWidth);
        const char *pszText = mEntry->Text();
        // Yes, the binary limits the number of characters by the width as well.
        const int nMaxChars = static_cast<int>(mEntry->mMaxEntryWidth);
        const KeyboardRequest request(this,
                                      pScreen,
                                      pszText,
                                      kKeyboardPad,
                                      nMaxChars,
                                      nMaxWidth,
                                      kNoFunctionKeys,
                                      kNoInvalidChars,
                                      kShowText,
                                      kSingleLine);
        pKeyboard->SetRequest(request);
        TheUI.GotoScreen(kKeyboardScreen);
    } else if (pMsg->mButton == kPadCross) {
        Submit();
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool SetupNameScreen::HandleTextEntryComplete([[maybe_unused]] UITextEntryCompleteMsg *pMsg) {
    Submit();
    return true;
}

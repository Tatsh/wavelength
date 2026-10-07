#include "met/uploadnotescreen.h"

#include <cstring>

#include "met/keyboardpanel.h"
#include "met/keyboardrequest.h"
#include "met/netdouploadscreen.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"
#include "ui/uitextentry.h"

namespace {

constexpr char kNotePanel[] = "fn_upload_n";
constexpr char kNoteEntry[] = "note";
constexpr char kUploadButton[] = "upload";
constexpr char kKeyboardPanel[] = "keyboard";
constexpr char kKeyboardScreen[] = "kb_screen";
constexpr char kUploadScreen[] = "fn_do_upload";
constexpr char kNoText[] = "";

// The keyboard request of the note.
constexpr int kKeyboardPad = 0;
constexpr int kNoCharLimit = 0;
constexpr int kNoFunctionKeys = 0;
constexpr int kNoInvalidChars = 0;
constexpr int kShowText = 0;

} // namespace

UploadNoteScreen::UploadNoteScreen(DataArray *pData) : FreqScreen(pData) {
}

void UploadNoteScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    UIComponent *pNote = TheUI.FindComponent(kNotePanel, kNoteEntry, false);
    mFocusPanel->SetFocus(pNote, kPadNone);
    pNote->SetText(mKeyboardText.c_str());
    static_cast<UITextEntry *>(pNote)->SetEditing(true);
    mKeyboardText = kNoText;
}

int UploadNoteScreen::ReceiveKeyboardText(const char *pszText) {
    mKeyboardText = pszText;
    return 1;
}

bool UploadNoteScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUITextEntryCompleteMsgType) {
        return HandleTextEntryComplete(static_cast<UITextEntryCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool UploadNoteScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    UIPanel *pPanel = mFocusPanel;
    if (pPanel == nullptr || std::strcmp(pPanel->mFocus->mName, kNoteEntry) != 0) {
        return FreqScreen::HandleJoypad(pMsg);
    }

    if (pMsg->mButton == kPadCircle) {
        if (pMsg->mPressed != 0) {
            KeyboardPanel *pKeyboard =
                dynamic_cast<KeyboardPanel *>(TheUI.FindPanel(kKeyboardPanel, false));
            UITextEntry *pNote =
                static_cast<UITextEntry *>(pPanel->FindComponent(kNoteEntry, false));
            UIScreen *pScreen = TheUI.mCurrentScreen;
            const KeyboardRequest request(this,
                                          pScreen,
                                          pNote->Text(),
                                          kKeyboardPad,
                                          kNoCharLimit,
                                          static_cast<int>(pNote->mMaxEntryWidth),
                                          kNoFunctionKeys,
                                          kNoInvalidChars,
                                          kShowText,
                                          pNote->mWordWrapLines);
            pKeyboard->SetRequest(request);
            TheUI.GotoScreen(kKeyboardScreen);
        }
    } else if (pMsg->mButton == kPadCross && pMsg->mPressed != 0) {
        mFocusPanel->SetFocus(TheUI.FindComponent(kNotePanel, kUploadButton, false), kPadNone);
        return true;
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool UploadNoteScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross && std::strcmp(pMsg->mComponent->mName, kUploadButton) == 0) {
        UIComponent *pNote = TheUI.FindComponent(kNotePanel, kNoteEntry, false);
        NetDoUploadScreen *pUpload =
            dynamic_cast<NetDoUploadScreen *>(TheUI.FindScreen(kUploadScreen, false));
        pUpload->mNote = pNote->Text();
        TheUI.GotoScreen(pUpload);
    }
    return UIScreen::HandleSelect(pMsg);
}

bool UploadNoteScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    UITextEntry *pFocus = dynamic_cast<UITextEntry *>(pMsg->mComponent);
    UITextEntry *pNote =
        dynamic_cast<UITextEntry *>(TheUI.FindComponent(kNotePanel, kNoteEntry, false));
    pNote->SetEditing(pFocus == pNote);
    return false;
}

bool UploadNoteScreen::HandleTextEntryComplete([[maybe_unused]] UITextEntryCompleteMsg *pMsg) {
    mFocusPanel->SetFocus(TheUI.FindComponent(kNotePanel, kUploadButton, false), kPadNone);
    return true;
}

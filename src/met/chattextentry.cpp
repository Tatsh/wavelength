#include "met/chattextentry.h"

#include "os/locale.h"
#include "os/string.h"

namespace {

constexpr char kFunctionKeyFormat[] = "fkey_f%d";

// The key codes of F1 to F12 run consecutively from F1.
constexpr int kKeyF1 = 401;
constexpr int kNumFunctionKeys = 12;

} // namespace

ChatTextEntry::ChatTextEntry(DataArray *pData, const char *pszPanel)
    : UITextEntry(pData, pszPanel) {
}

bool ChatTextEntry::HandleKeyboardKey(KeyboardKeyMsg *pMsg) {
    const int nIndex = pMsg->mKey - kKeyF1;
    if (nIndex >= 0 && nIndex < kNumFunctionKeys) {
        SetText(TheLocale.Localize(FormatString(kFunctionKeyFormat, nIndex + 1), true));
    }
    return HandleKeyMsg(pMsg);
}

bool ChatTextEntry::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nKeyboardKeyMsgType) {
        return HandleKeyboardKey(static_cast<KeyboardKeyMsg *>(pMsg));
    }
    return UITextEntry::DispatchPriv(pMsg);
}

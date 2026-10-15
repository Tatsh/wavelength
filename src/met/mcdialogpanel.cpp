#include "met/mcdialogpanel.h"

#include <cstring>

#include "met/errorscreen.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uimanager.h"

#ifdef VIDEO_STANDARD_PAL
MCDialogPanel::MCDialogPanel(DataArray *pData, const char *pszDir)
    : DialogPanel(pData, pszDir), mErrorMsg(1) {
    pData->FindBool("error_msg", &mErrorMsg, false);
}
#else
MCDialogPanel::MCDialogPanel(DataArray *pData, const char *pszDir) : DialogPanel(pData, pszDir) {
}
#endif

MCDialogPanel::~MCDialogPanel() {
}

void MCDialogPanel::Enter(bool bForce, float fTime) {
    DialogPanel::Enter(bForce, fTime);
    ShowSlotText();
}

void MCDialogPanel::ShowSlotText() {
    UIScreen *pCurrent = TheUI.mCurrentScreen;
    ErrorScreen *pScreen = pCurrent != nullptr ? dynamic_cast<ErrorScreen *>(pCurrent) : nullptr;
    String slot(pScreen->GetSlotName());
    // Where the binary passes only the slot, a text with a second %s (del_remix_check_dlg) reads
    // the stale a2 register, which the sorted locale lookup leaves 0. NameErrorScreen::Enter then
    // replaces the text.
#ifdef VIDEO_STANDARD_PAL
    String operation(pScreen->GetOperationText());
    String text;
    if (strcmp(operation.c_str(), "") == 0 || mErrorMsg == 0) {
        text = FormatString(TheLocale.Localize(mName, false), slot.c_str(), nullptr);
    } else {
        text = FormatString(TheLocale.Localize(mName, false), operation.c_str(), slot.c_str());
    }
#else
    String text(FormatString(TheLocale.Localize(mName, false), slot.c_str(), nullptr));
#endif
    SetText(text.c_str());
}

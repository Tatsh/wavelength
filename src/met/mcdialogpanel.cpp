#include "met/mcdialogpanel.h"

#include "met/errorscreen.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uimanager.h"

MCDialogPanel::MCDialogPanel(DataArray *pData, const char *pszDir) : DialogPanel(pData, pszDir) {
}

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
    String text(FormatString(TheLocale.Localize(mName, false), slot.c_str()));
    SetText(text.c_str());
}

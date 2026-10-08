#include "met/msgerrorscreen.h"

#include "met/dialogpanel.h"
#include "os/locale.h"

void MsgErrorScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    const String slot(GetSlotName());
    const String text(FormatString(TheLocale.Localize(mMessage.c_str(), false), slot.c_str()));
    dynamic_cast<DialogPanel *>(mFocusPanel)->SetText(text.c_str());
}

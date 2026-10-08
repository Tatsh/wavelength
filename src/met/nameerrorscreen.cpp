#include "met/nameerrorscreen.h"

#include "met/dialogpanel.h"
#include "os/locale.h"

void NameErrorScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    const String slot(GetSlotName());
    const String text(FormatString(
        TheLocale.Localize(mFocusPanel->mName, false), mSaveName.c_str(), slot.c_str()));
    dynamic_cast<DialogPanel *>(mFocusPanel)->SetText(text.c_str());
}

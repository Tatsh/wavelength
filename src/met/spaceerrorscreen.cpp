#include "met/spaceerrorscreen.h"

#include "met/dialogpanel.h"
#include "os/locale.h"
#include "os/string.h"

void SpaceErrorScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    const String slot(GetSlotName());
    const String text(
        FormatString(TheLocale.Localize(mFocusPanel->mName, false), slot.c_str(), mSpace));
    dynamic_cast<DialogPanel *>(mFocusPanel)->SetText(text.c_str());
}

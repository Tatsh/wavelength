#include "met/spaceerrorscreen.h"

#include "met/dialogpanel.h"
#include "os/locale.h"
#include "os/string.h"

void SpaceErrorScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    const String slot(GetSlotName());
#ifdef VIDEO_STANDARD_PAL
    const String text(FormatString(TheLocale.Localize(mFocusPanel->GetName(), false),
                                   slot.c_str(),
                                   mSpace,
                                   mSaveName.c_str()));
#else
    const String text(
        FormatString(TheLocale.Localize(mFocusPanel->GetName(), false), slot.c_str(), mSpace));
#endif
    dynamic_cast<DialogPanel *>(mFocusPanel)->SetText(text.c_str());
}

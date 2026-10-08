#include "met/transitionerrorscreen.h"

#include "met/dialogpanel.h"

void TransitionErrorScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    dynamic_cast<DialogPanel *>(mFocusPanel)->SetText(mMessage.c_str());
}

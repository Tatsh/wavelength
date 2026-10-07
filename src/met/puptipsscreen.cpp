#include "met/puptipsscreen.h"

#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "ui/uipanel.h"

PupTipsScreen::PupTipsScreen(DataArray *pData) : TipsScreen(pData) {
}

void PupTipsScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    TipsScreen::Enter(pPrevScreen, fTime);
    Rnd::Text *pText = dynamic_cast<Rnd::Text *>(
        Rnd::TheManager.Find(FormatString("%s_p_01.txt", mFocusPanel->mName)));
    if (pText != nullptr) {
        const String token(FormatString("%s_p_only", mFocusPanel->mName));
        pText->SetText(TheLocale.Localize(token.c_str(), true));
    }
}

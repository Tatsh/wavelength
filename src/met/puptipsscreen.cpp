#include "met/puptipsscreen.h"

#include "os/locale.h"
#include "os/string.h"
#include "rnd/rndmanager.h"
#include "rnd/rndtext.h"
#include "ui/uipanel.h"

PupTipsScreen::PupTipsScreen(DataArray *pData) : TipsScreen(pData) {
}

void PupTipsScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    TipsScreen::Enter(pPrevScreen, fTime);
    RndText *pText = dynamic_cast<RndText *>(
        TheManager.Find(FormatString("%s_p_01.txt", mFocusPanel->GetName())));
    if (pText != nullptr) {
        const String token(FormatString("%s_p_only", mFocusPanel->GetName()));
        pText->SetText(TheLocale.Localize(token.c_str(), true));
    }
}

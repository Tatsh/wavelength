#include "met/netwelcomescreen.h"

#include "os/joypad.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "ui/uipanel.h"

namespace {

constexpr char kNewsText[] = "fn_welcome_news_01.txt";
constexpr char kFirstButton[] = "01_but";

} // namespace

// Yes, the binary does not set mNews until NetServerLogin sets it.
NetWelcomeScreen::NetWelcomeScreen(DataArray *pData) : FreqScreen(pData) {
}

void NetWelcomeScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(kNewsText))->SetText(mNews);
    mFocusPanel->SetFocus(mFocusPanel->FindComponent(kFirstButton, false), kPadNone);
}

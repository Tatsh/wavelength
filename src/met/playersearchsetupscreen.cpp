#include "met/playersearchsetupscreen.h"

#include "met/netdoplayersearchscreen.h"
#include "ui/uimanager.h"

namespace {

constexpr char kSearchScreen[] = "fn_do_search";

} // namespace

void PlayerSearchSetupScreen::Submit() {
    NetDoPlayerSearchScreen *pSearch =
        dynamic_cast<NetDoPlayerSearchScreen *>(TheUI.FindScreen(kSearchScreen, false));
    pSearch->mPlayerName = mEntry->Text();
    TheUI.GotoScreen(pSearch);
}

#include "met/launchscreen.h"

#include "game/gamedb.h"
#include "ui/uimanager.h"

LaunchScreen::LaunchScreen(DataArray *pData) : FreqScreen(pData) {
}

void LaunchScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    UIScreen *pDialog;
    if (TheGameDb->mTutorial) {
        pDialog = TheUI.FindScreen("d_launch_training", false);
    } else {
        pDialog = TheUI.FindScreen("d_launch", false);
    }
    pDialog->Load();
    FreqScreen::Enter(pPrevScreen, fTime);
}

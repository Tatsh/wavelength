#include "met/mainoptionsscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "math/vector3.h"
#include "met/gizmo.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kCreditsOutScreen[] = "credits_out";
constexpr char kCreditsInScreen[] = "credits_in";
constexpr char kOnlineMainScreen[] = "fn_main_join";
constexpr char kMainScreen[] = "main";

// Where the projector waits while the credits show.
constexpr float kCreditsGizmoX = 800.0f;

} // namespace

bool MainOptionsScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void MainOptionsScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    if (strcmp(pNextScreen->GetName(), kCreditsInScreen) == 0) {
        const Vector3 position{kCreditsGizmoX, 0.0f, 0.0f};
        TheMetagame.GetGizmo()->MoveTo(position, mGizmoRot, fTime);
    }
}

void MainOptionsScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    if (strcmp(pPrevScreen->GetName(), kCreditsOutScreen) == 0) {
        TheMetagame.GetGizmo()->MoveTo(mGizmoOrig, mGizmoRot, fTime);
    }
}

bool MainOptionsScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->GetPressed()) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->GetButton() == kPadTriangle) {
            if (TheGameDb->GetCommunity() == GameDb::kCommunityOnline) {
                TheUI.GotoScreen(kOnlineMainScreen);
            } else {
                TheUI.GotoScreen(kMainScreen);
            }
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

#include "met/selloadedfreqscreen.h"

#include "game/gamedb.h"
#include "met/freqloadlist.h"
#include "met/gizmo.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanel[] = "f_load";
constexpr char kList[] = "list";

// Only the first controller exits the screen.
constexpr int kFirstPad = 0;

} // namespace

SelLoadedFreqScreen::SelLoadedFreqScreen(DataArray *pData) : FreqScreen(pData) {
}

void SelLoadedFreqScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    auto *pList = static_cast<FreqLoadList *>(TheUI.FindComponent(kPanel, kList, false));
    pList->SetProfiles(mProfiles);
    pList->SetSelected(0);
    if (mProfile.IsModified() != 0 && TheGameDb->GetProfile(0)->IsModified() == 0) {
        mProfile.SetModified(0);
    }
}

bool SelLoadedFreqScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool SelLoadedFreqScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        auto *pList = static_cast<UIList *>(TheUI.FindComponent(kPanel, kList, false));
        Campaign profile(mProfiles[pList->mSelected]);
        TheGameDb->ClearPlayers();
        TheGameDb->AddPlayer(&profile);
        TheMetagame.mGizmo->SetShowAvatar(true);
        if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
            TheUI.GotoScreen("solofreq2soloskill");
        } else {
            TheUI.GotoScreen("net_load_config");
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

bool SelLoadedFreqScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadTriangle && pMsg->mPad == kFirstPad) {
            if (TheGameDb->GetProfile(0)->mCustom != 0) {
                TheGameDb->ClearPlayers();
                TheGameDb->AddPlayer(&mProfile);
            }
            if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
                TheUI.GotoScreen("f_net_confirm");
            } else {
                TheUI.GotoScreen("f_confirm");
            }
        }
    }
    return UIScreen::HandleJoypad(pMsg);
}

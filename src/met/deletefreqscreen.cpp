#include "met/deletefreqscreen.h"

#include <cstring>

#include "game/campaign.h"
#include "game/gamedb.h"
#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/gizmo.h"
#include "met/metagame.h"
#include "os/debug.h"
#include "os/locale.h"
#include "ui/uimanager.h"

namespace {

constexpr char kDefaultNameToken[] = "player_1";

// The operation ErrorScreen::ShowCardErrorTwoOption() reports a failure of.
constexpr int kOperationDelete = 3;

// The slot whose deletion may remove the first player's Freq.
constexpr int kFirstSlot = 0;

// The first player.
constexpr int kFirstPlayer = 0;

} // namespace

void DeleteFreqScreen::OnFreqDeleted(int nStatus) {
    if (nStatus == MemcardTask::kStatusOk) {
        TheUI.GotoScreen(mDoneScreen.c_str());
    } else if (nStatus > MemcardTask::kStatusOk && nStatus < MemcardTask::kStatusExists) {
        ShowCardErrorTwoOption(nStatus, kOperationDelete);
    } else {
        DebugWarn("not handled delete freq! GET CHRISTINE\n");
    }
}

bool DeleteFreqScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool DeleteFreqScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) != 0) {
        return false;
    }
    if (mSlot == kFirstSlot && TheGameDb->GetProfile(kFirstPlayer)->mCustom != 0 &&
        std::strcmp(mFreqName.c_str(), TheGameDb->GetProfile(kFirstPlayer)->mName.c_str()) == 0) {
        TheGameDb->ClearPlayers();
        Campaign profile;
        profile.mName = TheLocale.Localize(kDefaultNameToken, true);
        TheGameDb->AddPlayer(&profile);
        TheMetagame.mGizmo->SetShowAvatar(false);
    }
    TheMCManager.DeleteFreq(this, mSlot, mFreqName.c_str());
    return false;
}

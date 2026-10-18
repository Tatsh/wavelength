#include "met/deletefreqscreen.h"

#include <cstring>

#include "game/campaign.h"
#include "game/gamedb.h"
#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/gizmo.h"
#include "met/metagame.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uimanager.h"

namespace {

constexpr char kDefaultNameToken[] = "player_1";

#ifdef VIDEO_STANDARD_PAL
// The dialog of a card without the Freq.
constexpr char kNoFreqsScreen[] = "no_freqs_loaded_error";
constexpr char kRetryComponent[] = "retry";
constexpr char kCancelComponent[] = "cancel";
#else
// The operation ErrorScreen::ShowCardErrorTwoOption() reports a failure of.
constexpr int kOperationDelete = 3;
#endif

// The slot whose deletion may remove the first player's Freq.
constexpr int kFirstSlot = 0;

// The first player.
constexpr int kFirstPlayer = 0;

} // namespace

#ifdef VIDEO_STANDARD_PAL
void DeleteFreqScreen::OnFreqDeleted(int nStatus) {
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        TheUI.GotoScreen(mDoneScreen.c_str());
        return;
    case MemcardTask::kStatusNoCard:
    case MemcardTask::kStatusFull:
    case MemcardTask::kStatusChangedCard:
        ShowCardErrorTwoOption(nStatus, kOperationDelete);
        return;
    case MemcardTask::kStatusUnformatted:
    case MemcardTask::kStatusNotFound: {
        auto *pError = dynamic_cast<ErrorScreen *>(TheUI.FindScreen(kNoFreqsScreen, false));
        pError->SetSlot(mSlot, kOperationLoad);
        pError->ClearTransitions();
        pError->AddTransition(kRetryComponent, kPadNone, mName);
        pError->AddTransition(kCancelComponent, kPadNone, mStartScreen.c_str());
        TheUI.GotoScreen(pError);
        return;
    }
    default:
        DebugWarn("not handled delete freq! GET CHRISTINE\n");
        return;
    }
}
#else
void DeleteFreqScreen::OnFreqDeleted(int nStatus) {
    if (nStatus == MemcardTask::kStatusOk) {
        TheUI.GotoScreen(mDoneScreen.c_str());
    } else if (nStatus > MemcardTask::kStatusOk && nStatus < MemcardTask::kStatusExists) {
        ShowCardErrorTwoOption(nStatus, kOperationDelete);
    } else {
        DebugWarn("not handled delete freq! GET CHRISTINE\n");
    }
}
#endif

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
        TheMetagame.GetGizmo()->SetShowAvatar(false);
    }
    TheMCManager.DeleteFreq(this, mSlot, mFreqName.c_str());
    return false;
}

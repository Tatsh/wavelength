#include "met/listremixesscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/remixselectscreen.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kMemRemixScreen[] = "mem_remix";
constexpr char kSoloRemixScreen[] = "soloremix2sololoadremix";
constexpr char kSoloGameScreen[] = "soloremix2sololoadgame";
constexpr char kLocalRemixScreen[] = "multiremix2multisong_LOAD";
constexpr char kLocalGameScreen[] = "multiloadcustom2multipickcustom";
constexpr char kNoRemixesScreen[] = "no_remixes_loaded_error";
constexpr char kFailedScreen[] = "load_failed_error";
constexpr char kRetryComponent[] = "retry";
constexpr char kCancelComponent[] = "cancel";

// The operation of ErrorScreen::ShowCardErrorTwoOption() for a failure outside a save, a copy, and
// a deletion.
constexpr int kNoOperation = 0;

// Go to an error dialog whose `retry` returns to the listing screen and whose `cancel` returns to
// the start screen.
inline void ShowListError(UIScreen *pError, const char *pszRetry, const char *pszCancel) {
    pError->ClearTransitions();
    pError->AddTransition(kRetryComponent, kPadNone, pszRetry);
    pError->AddTransition(kCancelComponent, kPadNone, pszCancel);
    TheUI.GotoScreen(pError);
}

} // namespace

void ListRemixesScreen::OnRemixesListed(int nStatus, std::vector<RemixInfo> *pInfos) {
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        if (!pInfos->empty()) {
            auto *pScreen =
                dynamic_cast<RemixSelectScreen *>(TheUI.FindScreen(mDoneScreen.c_str(), false));
            pScreen->SetRemixes(*pInfos);
            if (std::strcmp(pScreen->mName, kMemRemixScreen) == 0) {
                TheUI.GotoScreen(pScreen);
            } else if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
                TheUI.GotoScreen(TheGameDb->mRuleSet == GameDb::kRuleSetRemix ? kSoloRemixScreen :
                                                                                kSoloGameScreen);
            } else if (TheGameDb->mCommunity == GameDb::kCommunityLocal) {
                TheUI.GotoScreen(TheGameDb->mRuleSet == GameDb::kRuleSetRemix ? kLocalRemixScreen :
                                                                                kLocalGameScreen);
            } else {
                TheUI.GotoScreen(pScreen);
            }
            return;
        }
        break;
    case MemcardTask::kStatusNoCard:
    case MemcardTask::kStatusUnformatted:
    case MemcardTask::kStatusChangedCard:
        ShowCardErrorTwoOption(nStatus, kNoOperation);
        return;
    case MemcardTask::kStatusNotFound:
        break;
    default:
        ShowListError(TheUI.FindScreen(kFailedScreen, false), mName, mStartScreen.c_str());
        return;
    }

    auto *pError = dynamic_cast<ErrorScreen *>(TheUI.FindScreen(kNoRemixesScreen, false));
    pError->SetSlot(mSlot);
    ShowListError(pError, mName, mStartScreen.c_str());
}

bool ListRemixesScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool ListRemixesScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheMCManager.ListRemixes(this, mSlot);
    }
    return false;
}

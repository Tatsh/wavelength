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

#ifndef VIDEO_STANDARD_PAL
// The operation of ErrorScreen::ShowCardErrorTwoOption() for a failure outside a save, a copy, and
// a deletion.
constexpr int kNoOperation = 0;
#endif

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
            if (std::strcmp(pScreen->GetName(), kMemRemixScreen) == 0) {
                TheUI.GotoScreen(pScreen);
            } else if (TheGameDb->GetCommunity() == GameDb::kCommunitySolo) {
                TheUI.GotoScreen(TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix ?
                                     kSoloRemixScreen :
                                     kSoloGameScreen);
            } else if (TheGameDb->GetCommunity() == GameDb::kCommunityLocal) {
                TheUI.GotoScreen(TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix ?
                                     kLocalRemixScreen :
                                     kLocalGameScreen);
            } else {
                TheUI.GotoScreen(pScreen);
            }
            return;
        }
        break;
#ifdef VIDEO_STANDARD_PAL
    case MemcardTask::kStatusNoCard:
        ShowCardErrorTwoOption(nStatus, kOperationLoad);
        return;
    case MemcardTask::kStatusUnformatted:
    case MemcardTask::kStatusNotFound:
        break;
    case MemcardTask::kStatusChangedCard:
        mRelist = 1;
        return;
#else
    case MemcardTask::kStatusNoCard:
    case MemcardTask::kStatusUnformatted:
    case MemcardTask::kStatusChangedCard:
        ShowCardErrorTwoOption(nStatus, kNoOperation);
        return;
    case MemcardTask::kStatusNotFound:
        break;
#endif
    default:
        ShowListError(TheUI.FindScreen(kFailedScreen, false), mName, mStartScreen.c_str());
        return;
    }

    auto *pError = dynamic_cast<ErrorScreen *>(TheUI.FindScreen(kNoRemixesScreen, false));
#ifdef VIDEO_STANDARD_PAL
    pError->SetSlot(mSlot, kOperationSave); // Yes, the binary names a save here.
#else
    pError->SetSlot(mSlot);
#endif
    ShowListError(pError, mName, mStartScreen.c_str());
}

#ifdef VIDEO_STANDARD_PAL
void ListRemixesScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mRelist != 0) {
        mRelist = 0;
        TheMCManager.ListRemixes(this, mSlot);
    }
}
#endif

bool ListRemixesScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool ListRemixesScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->GetName(), mName) == 0) {
        TheMCManager.ListRemixes(this, mSlot);
    }
    return false;
}

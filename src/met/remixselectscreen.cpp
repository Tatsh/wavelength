#include "met/remixselectscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/loadremixscreen.h"
#include "met/remixloadlist.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kGreyReadOnlyTag[] = "grey_read_only";
constexpr char kGreyUnplayableTag[] = "grey_unplayable";
constexpr char kListComponent[] = "list";
constexpr char kOnlineLoadScreen[] = "net_custom_load";
constexpr char kLoadRemixScreen[] = "load_remix";
constexpr char kSoloDoneScreen[] = "pre_solotut2launchseq";
constexpr char kSoloGameStartScreen[] = "s_g_custom_load";
constexpr char kSoloRemixStartScreen[] = "s_r_load";
constexpr char kLocalDoneScreen[] = "pre_multitut2launchseq";
constexpr char kLocalGameStartScreen[] = "m_g_custom_load";
constexpr char kLocalRemixStartScreen[] = "m_r_load";
constexpr char kLaunchpadScreen[] = "fn_h_lpad";
constexpr char kHostAttemptScreen[] = "net_host_attempt";
constexpr char kRemixReadOnlyCheckScreen[] = "net_remix_read_only_check";
constexpr char kReadOnlyCheckScreen[] = "net_read_only_check";
constexpr char kReadOnlyWarningScreen[] = "net_share_disc_read_only_warning";
constexpr char kEditHostScreen[] = "edit_host";
constexpr char kHostingScreen[] = "fn_hosting";
constexpr char kNoCreator[] = "";

// The values LoadSelectedRemix() reports.
constexpr int kLoadGreyed = 1;
constexpr int kLoadChecked = 0;
constexpr int kLoadStarted = -1;

// The controller that returns to the host screen.
constexpr int kFirstPad = 0;

} // namespace

RemixSelectScreen::RemixSelectScreen(DataArray *pData)
    : FreqScreen(pData), mGreyReadOnly(0), mGreyUnplayable(0) {
    pData->FindBool(kGreyReadOnlyTag, &mGreyReadOnly, false);
    pData->FindBool(kGreyUnplayableTag, &mGreyUnplayable, false);
}

void RemixSelectScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    auto *pList = static_cast<RemixLoadList *>(
        TheUI.FindComponent(mFocusPanel->mName, kListComponent, false));
    if (std::strcmp(mName, kOnlineLoadScreen) == 0) {
        mGreyReadOnly = 0;
        mGreyUnplayable = TheGameDb->mRuleSet != GameDb::kRuleSetRemix;
    }
    pList->SetRemixes(mRemixes, mGreyReadOnly, mGreyUnplayable);
    pList->SetSelected(0);
}

int RemixSelectScreen::LoadSelectedRemix(bool bPractice) {
    UIComponent *pList = TheUI.FindComponent(mFocusPanel->mName, kListComponent, false);
    RemixInfo info = mRemixes[static_cast<UIList *>(pList)->mSelected];
    if ((mGreyReadOnly != 0 && info.mReadOnly != 0) ||
        (mGreyUnplayable != 0 && info.mPlayable == 0)) {
        FxMidi::PlayWrong();
        return kLoadGreyed;
    }
    if (bPractice) {
        FxMidi::PlaySquare();
    }

    TheGameDb->SetSong(info.mSong);
    TheGameDb->SetPracticeMode(bPractice);
    TheGameDb->SetTutorial(0);
    TheGameDb->SetRemixActive(1);
    TheGameDb->SetLoadRemix(true);
    TheGameDb->SetSkillLevel(info.mSkillLevel);
    TheGameDb->SetRemixName(info.mName);
    TheGameDb->SetRemixReadOnly(info.mReadOnly != 0);
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix &&
        TheGameDb->mCommunity != GameDb::kCommunityOnline && info.mReadOnly == 0) {
        for (int i = 0; i < kRemixInfoCreatorCount; ++i) {
            const char *pszCreator =
                i < TheGameDb->GetNumPlayers() ? TheGameDb->GetPlayerName(i) : kNoCreator;
            std::strncpy(info.mCreators[i], pszCreator, kRemixInfoCreatorSize - 1);
        }
    }
    TheGameDb->SetRemix(&info);

    auto *pLoad = dynamic_cast<LoadRemixScreen *>(TheUI.FindScreen(kLoadRemixScreen, false));
    const int nCommunity = TheGameDb->mCommunity;
    if (nCommunity == GameDb::kCommunitySolo) {
        pLoad->SetDoneScreen(kSoloDoneScreen);
        pLoad->SetStartScreen(TheGameDb->mRuleSet == GameDb::kRuleSetGame ? kSoloGameStartScreen :
                                                                            kSoloRemixStartScreen);
    } else if (nCommunity == GameDb::kCommunityLocal) {
        pLoad->SetDoneScreen(kLocalDoneScreen);
        pLoad->SetStartScreen(TheGameDb->mRuleSet == GameDb::kRuleSetGame ? kLocalGameStartScreen :
                                                                            kLocalRemixStartScreen);
    } else {
        pLoad->SetStartScreen(kOnlineLoadScreen);
        pLoad->SetDoneScreen(TheNetLaunchpad != nullptr ? kLaunchpadScreen : kHostAttemptScreen);
        if (info.mReadOnly == 0) {
            TheUI.GotoScreen(TheGameDb->mRuleSet == GameDb::kRuleSetRemix ?
                                 kRemixReadOnlyCheckScreen :
                                 kReadOnlyCheckScreen);
            return kLoadChecked;
        }
        if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
            TheUI.GotoScreen(kReadOnlyWarningScreen);
            return kLoadChecked;
        }
    }
    TheUI.GotoScreen(pLoad);
    return kLoadStarted;
}

bool RemixSelectScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool RemixSelectScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        const int nResult = LoadSelectedRemix(false);
        if (nResult != kLoadStarted) {
            return nResult != 0;
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

bool RemixSelectScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return UIScreen::HandleJoypad(pMsg);
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    if (pMsg->mButton == kPadTriangle && pMsg->mPad == kFirstPad &&
        TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        TheUI.GotoScreen(TheNetLaunchpad != nullptr ? kEditHostScreen : kHostingScreen);
    } else if (pMsg->mButton == kPadSquare && TheGameDb->mCommunity == GameDb::kCommunitySolo &&
               TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        const int nResult = LoadSelectedRemix(true);
        if (nResult != kLoadStarted) {
            return nResult != 0;
        }
    }
    return UIScreen::HandleJoypad(pMsg);
}

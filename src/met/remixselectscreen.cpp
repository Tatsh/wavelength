#include "met/remixselectscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/loadremixscreen.h"
#include "met/remixloadlist.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"

namespace {

constexpr char kListComponent[] = "list";
constexpr char kNetLoadScreen[] = "net_custom_load";

// The longest player name strncpy() copies into a RemixInfo.
constexpr size_t kCreatorCopyLength = 15;

// Only the first controller goes back from the online list.
constexpr int kFirstPad = 0;

// LoadSelected() reports these.
constexpr int kRefused = 1;
constexpr int kQuestion = 0;
constexpr int kLoading = -1;

} // namespace

RemixSelectScreen::RemixSelectScreen(DataArray *pData) : FreqScreen(pData) {
    mGreyReadOnly = 0;
    mGreyUnplayable = 0;
    bool bGrey = false;
    if (pData->FindBool("grey_read_only", &bGrey, false)) {
        mGreyReadOnly = bGrey;
    }
    bGrey = false;
    if (pData->FindBool("grey_unplayable", &bGrey, false)) {
        mGreyUnplayable = bGrey;
    }
}

void RemixSelectScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    auto *pList = static_cast<RemixLoadList *>(
        TheUI.FindComponent(mFocusPanel->mName, kListComponent, false));
    if (strcmp(mName, kNetLoadScreen) == 0) {
        mGreyReadOnly = 0;
        mGreyUnplayable = TheGameDb->mRuleSet == GameDb::kRuleSetRemix ? 0 : 1;
    }
    pList->SetRemixes(mRemixes, mGreyReadOnly, mGreyUnplayable);
    pList->SetSelected(0);
}

int RemixSelectScreen::LoadSelected(bool bPractice) {
    auto *pList = static_cast<RemixLoadList *>(
        TheUI.FindComponent(mFocusPanel->mName, kListComponent, false));
    RemixInfo info = mRemixes[pList->mSelected];
    if ((mGreyReadOnly != 0 && info.mReadOnly != 0) ||
        (mGreyUnplayable != 0 && info.mPlayable == 0)) {
        FxMidi::PlayWrong();
        return kRefused;
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
                i < TheGameDb->GetNumPlayers() ? TheGameDb->GetPlayerName(i) : "";
            strncpy(info.mCreators[i], pszCreator, kCreatorCopyLength);
        }
    }
    TheGameDb->SetRemix(&info);

    auto *pLoad = dynamic_cast<LoadRemixScreen *>(TheUI.FindScreen("load_remix", false));
    const bool bGame = TheGameDb->mRuleSet == GameDb::kRuleSetGame;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        pLoad->SetDoneScreen("pre_solotut2launchseq");
        pLoad->SetStartScreen(bGame ? "s_g_custom_load" : "s_r_load");
    } else if (TheGameDb->mCommunity == GameDb::kCommunityLocal) {
        pLoad->SetDoneScreen("pre_multitut2launchseq");
        pLoad->SetStartScreen(bGame ? "m_g_custom_load" : "m_r_load");
    } else {
        pLoad->SetStartScreen(kNetLoadScreen);
        pLoad->SetDoneScreen(TheNetLaunchpad != nullptr ? "fn_h_lpad" : "net_host_attempt");
        const bool bRemix = TheGameDb->mRuleSet == GameDb::kRuleSetRemix;
        if (info.mReadOnly == 0) {
            TheUI.GotoScreen(bRemix ? "net_remix_read_only_check" : "net_read_only_check");
            return kQuestion;
        }
        if (bRemix) {
            TheUI.GotoScreen("net_share_disc_read_only_warning");
            return kQuestion;
        }
    }
    TheUI.GotoScreen(pLoad);
    return kLoading;
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
        const int nResult = LoadSelected(false);
        if (nResult != kLoading) {
            return nResult != 0;
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

bool RemixSelectScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadTriangle && pMsg->mPad == kFirstPad &&
            TheGameDb->mCommunity == GameDb::kCommunityOnline) {
            TheUI.GotoScreen(TheNetLaunchpad != nullptr ? "edit_host" : "fn_hosting");
            return UIScreen::HandleJoypad(pMsg);
        }
    }
    if (pMsg->mPressed != 0 && pMsg->mButton == kPadSquare &&
        TheGameDb->mCommunity == GameDb::kCommunitySolo &&
        TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        const int nResult = LoadSelected(true);
        if (nResult != kLoading) {
            return nResult != 0;
        }
    }
    return UIScreen::HandleJoypad(pMsg);
}

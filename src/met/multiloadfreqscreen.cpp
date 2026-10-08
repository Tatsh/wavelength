#include "met/multiloadfreqscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "math/rand.h"
#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/freqselpanel.h"
#include "met/mcdialogpanel.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "ui/uimanager.h"

namespace {

constexpr char kDuelStartScreen[] = "m_mode";
constexpr char kStartScreen[] = "m_player";
constexpr char kDialogPanel[] = "multi_load_freq_dlg";
constexpr char kMetagameEntry[] = "metagame";
constexpr char kPrefabsEntry[] = "prefabs";
constexpr char kSelectPanelFormat[] = "m_g_s_f_%dpl_0%d";
constexpr char kDuelSelectScreen[] = "f_m_sel_2_duel";
constexpr char kSelectScreenFormat[] = "f_m_sel_%d";

// The value of mSelected and of mFirstProfile for none.
constexpr int kNone = -1;

// The first entry of the prefabs after the tag.
constexpr int kFirstPrefab = 1;

// The first player, whose Freq starts on the Freq of the player's name.
constexpr int kFirstPlayer = 0;

// The first memory card slot.
constexpr int kFirstSlot = 0;

} // namespace

void MultiLoadFreqScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        mStartScreen = kDuelStartScreen;
    } else {
        mStartScreen = kStartScreen;
    }
    FreqScreen::Enter(pPrevScreen, fTime);
}

void MultiLoadFreqScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mLoaded != 0) {
        mLoaded = 0;
        LoadNextSlot();
    }
}

void MultiLoadFreqScreen::LoadNextSlot() {
    if (mSlot + 1 < TheGameDb->GetNumPlayers()) {
        ++mSlot;
        dynamic_cast<MCDialogPanel *>(TheUI.FindPanel(kDialogPanel, false))->ShowSlotText();
        TheMCManager.LoadFreqs(this, mSlot);
        return;
    }

    const int nLoaded = static_cast<int>(mProfiles.size());
    DataArray *pPrefabs =
        SystemConfig()->FindArray(kMetagameEntry, true)->FindArray(kPrefabsEntry, true);
    const int nPrefabs = pPrefabs->Size();
    Campaign prefab;
    for (int i = kFirstPrefab; i < nPrefabs; ++i) {
        prefab.mAvatar.Load(pPrefabs->Array(i));
        prefab.mName = TheLocale.Localize(pPrefabs->Array(i)->Sym(0), true);
        mProfiles.push_back(prefab);
    }

    for (int nPlayer = 0; nPlayer < TheGameDb->GetNumPlayers(); ++nPlayer) {
        if (nPlayer == kFirstPlayer && mSelected[kFirstPlayer] != kNone) {
            if (TheGameDb->GetProfile(kFirstPlayer)->mCustom == 0) {
                continue;
            }
            mFirstProfile[kFirstPlayer] = 0;
            const int nCount = mFirstProfile[kFirstPlayer + 1] != kNone ?
                                   mFirstProfile[kFirstPlayer + 1] :
                                   nLoaded;
            for (int i = 0; i < nCount; ++i) {
                if (std::strcmp(mProfiles[i].mName.c_str(),
                                TheGameDb->GetPlayerName(kFirstPlayer)) == 0) {
                    mSelected[kFirstPlayer] = i;
                    break;
                }
            }
        } else if (mSelected[nPlayer] == kNone) {
            mSelected[nPlayer] = RandomInt(nLoaded, static_cast<int>(mProfiles.size()));
        }
    }

    for (int nPlayer = 0; nPlayer < TheGameDb->GetNumPlayers(); ++nPlayer) {
        auto *pPanel = static_cast<FreqSelPanel *>(TheUI.FindPanel(
            FormatString(kSelectPanelFormat, TheGameDb->GetNumPlayers(), nPlayer + 1), false));
        pPanel->SetProfiles(mProfiles, mSelected[nPlayer]);
    }
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        TheUI.GotoScreen(kDuelSelectScreen);
    } else {
        TheUI.GotoScreen(FormatString(kSelectScreenFormat, TheGameDb->GetNumPlayers()));
    }
    mProfiles.clear();
    mSlot = 0;
}

void MultiLoadFreqScreen::OnFreqsLoaded(int nStatus, std::vector<Campaign> *pProfiles) {
    if (nStatus == MemcardTask::kStatusChangedCard) {
        mLoaded = 1;
        --mSlot; // The next poll reads the slot again.
        return;
    }
    if (nStatus == MemcardTask::kStatusOk && !pProfiles->empty()) {
        mFirstProfile[mSlot] = static_cast<int>(mProfiles.size());
        mSelected[mSlot] = static_cast<int>(mProfiles.size());
        for (unsigned int i = 0; i < pProfiles->size(); ++i) {
            mProfiles.push_back((*pProfiles)[i]);
        }
    }
    mLoaded = 1;
}

bool MultiLoadFreqScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool MultiLoadFreqScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        mProfiles.clear();
        for (int i = 0; i < kMaxPlayers; ++i) {
            mFirstProfile[i] = kNone;
            mSelected[i] = kNone;
        }
        mSlot = kFirstSlot;
        TheMCManager.LoadFreqs(this, kFirstSlot);
    }
    return false;
}

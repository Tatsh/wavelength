#include "met/freqcopydelscreen.h"

#include <string.h>

#include "game/campaign.h"
#include "game/gamedb.h"
#include "met/copyfreqscreen.h"
#include "met/deletefreqscreen.h"
#include "met/nameerrorscreen.h"
#include "os/joypad.h"
#include "os/string.h"
#include "synth/fxmidi.h"
#include "ui/uilist.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanel[] = "f_load";
constexpr char kListComponent[] = "list";

// The player whose Freq a deletion may not remove while it is in use.
constexpr int kFirstPlayer = 0;

} // namespace

FreqCopyDelScreen::FreqCopyDelScreen(DataArray *pData) : SelLoadedFreqScreen(pData) {
}

bool FreqCopyDelScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return SelLoadedFreqScreen::DispatchPriv(pMsg);
}

void FreqCopyDelScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    SelLoadedFreqScreen::Enter(pPrevScreen, fTime);
}

bool FreqCopyDelScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    static_cast<UIList *>(TheUI.FindComponent(kPanel, kListComponent, false))
        ->SetCursorSelected(true);
    return UIScreen::HandleSelect(pMsg);
}

bool FreqCopyDelScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return UIScreen::HandleJoypad(pMsg);
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    {
        UIList *pList = static_cast<UIList *>(TheUI.FindComponent(kPanel, kListComponent, false));
        Campaign profile(mProfiles[pList->mSelected]);
        if (pMsg->mButton == kPadCircle) {
            const int nOtherSlot = mSlot == 0;
            CopyFreqScreen *pCopy =
                dynamic_cast<CopyFreqScreen *>(TheUI.FindScreen("copy_freq", false));
            pCopy->SetSlot(nOtherSlot);
            pCopy->mOverwriteStatus = 0;
            pCopy->mProfile = profile;
            TheUI.GotoScreen(pCopy);
        } else if (pMsg->mButton == kPadSquare) {
            String check("del_freq_check");
            if (mSlot == 0 && TheGameDb->GetProfile(kFirstPlayer)->mCustom != 0 &&
                strcmp(profile.mName.c_str(), TheGameDb->GetProfile(kFirstPlayer)->mName.c_str()) ==
                    0) {
                if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
                    FxMidi::PlayWrong();
                    TheUI.GotoScreen("no_del_freq_net");
                    return UIScreen::HandleJoypad(pMsg);
                }
                if (TheGameDb->GetProfile(kFirstPlayer)->IsModified() != 0) {
                    UIScreen *pSave = TheUI.FindScreen("cur_freq_not_saved", false);
                    pSave->ClearTransitions();
                    pSave->AddTransition("save", kPadNone, "del_freq");
                    pSave->AddTransition("continue", kPadNone, "del_freq");
                    pSave->AddTransition("cancel", kPadNone, mName);
                    check = "del_freq_check_needs_save";
                }
            }
            FxMidi::PlaySquare();
            DeleteFreqScreen *pDelete =
                dynamic_cast<DeleteFreqScreen *>(TheUI.FindScreen("del_freq", false));
            pDelete->mFreqName = profile.mName.c_str();
            pDelete->SetSlot(mSlot);
            pDelete->SetStartScreen("mem_freqs");
            if (mProfiles.size() == 1) {
                pDelete->SetDoneScreen("o_rf_mem");
            } else {
                pDelete->SetDoneScreen("mc_load_freq");
            }
            NameErrorScreen *pCheck =
                dynamic_cast<NameErrorScreen *>(TheUI.FindScreen(check.c_str(), false));
            pCheck->SetSlot(mSlot);
            pCheck->mSaveName = profile.mName.c_str();
            TheUI.GotoScreen(pCheck);
        }
    }
    return UIScreen::HandleJoypad(pMsg);
}

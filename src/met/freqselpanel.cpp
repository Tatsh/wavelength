#include "met/freqselpanel.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/metagameutil.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "synth/fxmidi.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uinavigator.h"

namespace {

constexpr char kAvatarIndexEntry[] = "avatar_index";
constexpr char kPanelMeshFormat[] = "%s_panel.mesh";
constexpr char kFreqMeshFormat[] = "%s_freq.mesh";
constexpr char kLeftArrowFormat[] = "%s_but_left.mesh";
constexpr char kRightArrowFormat[] = "%s_but_right.mesh";
constexpr char kRankMeshFormat[] = "%s_r.mesh";
constexpr char kChosenTabMat[] = "panel_tab_hi.mat";
constexpr char kChosenFreqMat[] = "freq_sel.mat";
constexpr char kTabMat[] = "panel_tab.mat";
constexpr char kFreqMat[] = "freq.mat";
constexpr char kNameButtonFormat[] = "but%d";
constexpr char kDefaultNameFormat[] = "default_name_%d";
constexpr char kNoName[] = "";

constexpr int kFirstPlayer = 0;
constexpr int kNoRankIcon = -1;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

template <typename T>
T *FindObject(const char *pszFormat, const char *pszPanel) {
    return FindObject<T>(FormatString(pszFormat, pszPanel));
}

} // namespace

FreqSelPanel::FreqSelPanel(DataArray *pData, const char *pszDir)
    : AvatarPanel(pData, pszDir), mAvatarIndex(0), mNumProfiles(0), mSelected(1) {
    pData->FindInt(kAvatarIndexEntry, &mAvatarIndex, false);
}

void FreqSelPanel::SetProfiles(const std::vector<Campaign> &profiles, int nSelected) {
    SetAvatar(nullptr);
    mProfiles.clear();
    mProfiles = profiles;
    mSelected = nSelected;
}

void FreqSelPanel::Enter(bool bForce, float fTime) {
    mNumProfiles = static_cast<int>(mProfiles.size());
    if (TheGameDb->mCommunity == GameDb::kCommunityLocal && TheUI.FocusPanel() == this) {
        if (mAvatarIndex != kFirstPlayer || TheGameDb->GetProfile(kFirstPlayer)->mCustom == 0) {
            *TheGameDb->GetProfile(mAvatarIndex) = mProfiles[mSelected];
        }
    }
    AvatarPanel::Enter(bForce, fTime);
    SetChosen(false);
    ShowSelected();
}

void FreqSelPanel::SetChosen(bool bChosen) {
    Rnd::Mesh *pPanelMesh = FindObject<Rnd::Mesh>(kPanelMeshFormat, mName);
    pPanelMesh->SetMat(FindObject<Rnd::Mat>(bChosen ? kChosenTabMat : kTabMat));
    Rnd::Mesh *pFreqMesh = FindObject<Rnd::Mesh>(kFreqMeshFormat, mName);
    pFreqMesh->SetMat(FindObject<Rnd::Mat>(bChosen ? kChosenFreqMat : kFreqMat));
    FindObject<Rnd::Mesh>(kLeftArrowFormat, mName)->SetShowing(!bChosen);
    FindObject<Rnd::Mesh>(kRightArrowFormat, mName)->SetShowing(!bChosen);
}

void FreqSelPanel::Unload() {
    FreqPanel::Unload();
    mProfiles.clear();
}

bool FreqSelPanel::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return UIPanel::DispatchPriv(pMsg);
}

bool FreqSelPanel::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    if (pMsg->mPad != mAvatarIndex) {
        return false;
    }

    bool bMoved = false;
    if (pMsg->mButton == kPadDLeft) {
        bMoved = true;
        mSelected = mSelected - 1 > -1 ? mSelected - 1 : mNumProfiles - 1;
        FxMidi::PlayMenuLeft();
    } else if (pMsg->mButton == kPadDRight) {
        bMoved = true;
        mSelected = mSelected + 1 < mNumProfiles ? mSelected + 1 : 0;
        FxMidi::PlayMenuRight();
    }

    if (pMsg->mButton == kPadCross) {
        *TheGameDb->GetProfile(mAvatarIndex) = mProfiles[mSelected];
        FxMidi::PlayMenuSelect();
        return false;
    }

    ShowSelected();
    if (bMoved) {
        mNavigator->Dispatch(pMsg);
    }
    return bMoved;
}

bool FreqSelPanel::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        SetChosen(true);
    }
    return false;
}

void FreqSelPanel::ShowSelected() {
    Campaign &profile = mProfiles[mSelected];
    SetAvatar(&profile.mAvatar);
    Rnd::Mesh *pRankMesh = FindObject<Rnd::Mesh>(kRankMeshFormat, mName);
    const String button(FormatString(kNameButtonFormat, mAvatarIndex + 1));
    if (std::strcmp(profile.mName.c_str(), kNoName) == 0) {
        FindComponent(button.c_str(), false)
            ->SetText(TheLocale.Localize(FormatString(kDefaultNameFormat, mAvatarIndex + 1), true));
        pRankMesh->SetMat(FindRankMaterial(kNoRankIcon));
    } else {
        FindComponent(button.c_str(), false)->SetText(profile.mName.c_str());
        pRankMesh->SetMat(FindRankMaterial(profile.GetHighestBeatenSkillLevel()));
    }
}

#include "met/netlpadplayerpanel.h"

#include <cstring>

#include "met/bootscreen.h"
#include "met/metagame.h"
#include "met/metagameutil.h"
#include "met/netlpadgamepanel.h"
#include "met/netlpadscreen.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "synth/fxmidi.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uiscreen.h"

namespace {

constexpr char kFirstPlayerButton[] = "player_1";
constexpr char kPlayerButtonFormat[] = "player_%d";
constexpr char kGamesComponent[] = "games";
constexpr char kNetComponent[] = "net";
constexpr char kGamesToken[] = "fn_h_lpad_play_05";
constexpr char kNetToken[] = "fn_h_lpad_play_06";
constexpr char kColorMesh[] = "fn_h_lpad_play_05c.mesh";
constexpr char kPanelMeshFormat[] = "%s_panel.mesh";
constexpr char kBgMeshFormat[] = "%s_bg.mesh";
constexpr char kTabHiliteMat[] = "panel_sub_hi.mat";
constexpr char kTabMat[] = "panel_sub.mat";
constexpr char kBgHiliteMat[] = "bg_hi.mat";
constexpr char kBgMat[] = "bg_no.mat";
constexpr char kFocusActionToken[] = "fn_g_lpad_play_focus_ACTION";
constexpr char kPanelHelpToken[] = "fn_g_lpad_player_HELP";
constexpr char kPanelActionToken[] = "fn_g_lpad_play_ACTION";
constexpr char kHostHelpToken[] = "lpad_h_p_HELP";
constexpr char kGuestHelpToken[] = "lpad_g_p_HELP";
constexpr char kBootScreen[] = "fn_boot_player";

constexpr int kHostId = 0;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

void SetMeshMat(const char *pszMeshFormat, const char *pszPanel, const char *pszMat) {
    Rnd::Mesh *pMesh = FindObject<Rnd::Mesh>(FormatString(pszMeshFormat, pszPanel));
    pMesh->SetMat(FindObject<Rnd::Mat>(pszMat));
}

// Find the player at a position of the list, or the end of the list.
std::list<NetLaunchpadPlayer>::iterator FindPlayer(std::list<NetLaunchpadPlayer> &players,
                                                   int nIndex) {
    auto player = players.begin();
    for (int i = 0; player != players.end() && i < nIndex; ++i) {
        ++player;
    }
    return player;
}

} // namespace

void NetLPadPlayerPanel::Enter(bool bForce, float fTime) {
    AvatarPanel::Enter(bForce, fTime);
    mSelected = 0;
    SetFocus(FindComponent(kFirstPlayerButton, false), kPadNone);
    Unfocus();
}

void NetLPadPlayerPanel::Update(std::list<NetLaunchpadPlayer> *pPlayers) {
    String focus;
    if (mFocus != nullptr) {
        focus = mFocus->Text();
    }
    NetLPadGamePanel::ShowPlayers(mName, pPlayers);
    bool bFound = false;
    if (TheNetLaunchpad == nullptr) {
        return;
    }

    auto player = pPlayers->begin();
    for (int nRow = 1; nRow <= kNumPlayerButtons; ++nRow) {
        UIComponent *pButton = FindComponent(FormatString(kPlayerButtonFormat, nRow), false);
        if (pPlayers->size() < static_cast<unsigned int>(nRow)) {
            pButton->SetState(UIComponent::kStateDisabled, false);
        } else {
            pButton->SetState(UIComponent::kStateNormal, false);
        }
        if (std::strcmp(pButton->Text(), focus.c_str()) == 0) {
            mSelected = nRow - 1;
            bFound = true;
            SetAvatar(&player->mPlayer.mAvatar);
            ShowPlayer(&*player);
            SetFocus(pButton, kPadNone);
            if (TheUI.mCurrentScreen->mFocusPanel == this) {
                mFocus->SetState(UIComponent::kStateSelected, false);
            } else {
                mFocus->SetState(UIComponent::kStateNormal, false);
            }
        }
        if (player != pPlayers->end()) {
            ++player;
        }
    }

    if (!bFound && !pPlayers->empty()) {
        mSelected = 0;
        player = pPlayers->begin();
        SetAvatar(&player->mPlayer.mAvatar);
        ShowPlayer(&*player);
        SetFocus(FindComponent(kFirstPlayerButton, false), kPadNone);
        if (TheUI.mCurrentScreen->mFocusPanel == this) {
            mFocus->SetState(UIComponent::kStateSelected, false);
        } else {
            mFocus->SetState(UIComponent::kStateNormal, false);
        }
    }
}

void NetLPadPlayerPanel::ShowPlayer(NetLaunchpadPlayer *pPlayer) {
    FindComponent(kGamesComponent, false)
        ->SetText(FormatString(TheLocale.Localize(kGamesToken, true), pPlayer->mPlayer.mGames));
    UIComponent *pNet = FindComponent(kNetComponent, false);
    const String connection(GetConnectionTypeName(pPlayer->mPlayer.mConnectionType));
    pNet->SetText(FormatString(TheLocale.Localize(kNetToken, true), connection.c_str()));
    const char *pszColor = GetPlayerColorName(pPlayer->mDifficulty);
    Rnd::Mesh *pColor = FindObject<Rnd::Mesh>(kColorMesh);
    pColor->SetMat(FindColorMaterial(pszColor));
}

void NetLPadPlayerPanel::Focus() {
    if (!mLoaded) {
        return;
    }
    UIPanel::Focus();
    SetMeshMat(kPanelMeshFormat, mName, kTabHiliteMat);
    SetMeshMat(kBgMeshFormat, mName, kBgHiliteMat);
    mFocus->SetState(UIComponent::kStateSelected, true);
    TheMetagame.SetActionText(TheLocale.Localize(kFocusActionToken, true));
    UpdateHelp();
}

void NetLPadPlayerPanel::Unfocus() {
    SetMeshMat(kPanelMeshFormat, mName, kTabMat);
    SetMeshMat(kBgMeshFormat, mName, kBgMat);
    if (mFocus != nullptr) {
        mFocus->SetState(UIComponent::kStateNormal, true);
    }
    TheMetagame.SetHelpText(TheLocale.Localize(kPanelHelpToken, true));
    TheMetagame.SetActionText(TheLocale.Localize(kPanelActionToken, true));
}

void NetLPadPlayerPanel::UpdateHelp() {
    NetLpadScreen *pScreen = dynamic_cast<NetLpadScreen *>(TheUI.mCurrentScreen);
    auto player = FindPlayer(pScreen->mPlayers, mSelected);
    if (TheNetLaunchpad != nullptr && TheNetLaunchpad->IsGuest() == kHostId &&
        player->mId != TheNetLaunchpad->IsGuest()) {
        TheMetagame.SetHelpText(TheLocale.Localize(kHostHelpToken, true));
    } else {
        TheMetagame.SetHelpText(TheLocale.Localize(kGuestHelpToken, true));
    }
}

bool NetLPadPlayerPanel::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    return UIPanel::DispatchPriv(pMsg);
}

bool NetLPadPlayerPanel::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    if (pMsg->mComponent != nullptr && TheUI.mCurrentScreen != nullptr &&
        TheUI.mCurrentScreen->mFocusPanel == this) {
        UpdateHelp();
    }
    return false;
}

bool NetLPadPlayerPanel::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    UIScreen *pCurrent = TheUI.mCurrentScreen;
    if (pCurrent->mNextScreen != nullptr || pCurrent->mPrevScreen != nullptr) {
        return true;
    }

    NetLpadScreen *pScreen = dynamic_cast<NetLpadScreen *>(pCurrent);
    std::list<NetLaunchpadPlayer> &players = pScreen->mPlayers;
    const int nButton = pMsg->mButton;
    if (nButton == kPadTriangle || nButton == kPadDLeft) {
        pScreen->SetFocus(TheUI.FindPanel(pScreen->mButtonPanel, false));
        return false;
    }
    if (nButton == kPadDUp || nButton == kPadDDown) {
        const int nCount = static_cast<int>(players.size());
        if (nButton == kPadDUp) {
            mSelected = mSelected - 1 > -1 ? mSelected - 1 : nCount - 1;
        } else {
            mSelected = mSelected + 1 < nCount ? mSelected + 1 : 0;
        }
        auto player = FindPlayer(players, mSelected);
        SetAvatar(&player->mPlayer.mAvatar);
        ShowPlayer(&*player);
        return false;
    }

    if (TheNetLaunchpad != nullptr && TheNetLaunchpad->IsGuest() == kHostId &&
        nButton == kPadSquare) {
        auto player = FindPlayer(players, mSelected);
        if (player->mId == TheNetLaunchpad->IsGuest()) {
            FxMidi::PlayWrong();
            return false;
        }
        FxMidi::PlaySquare();
        NetLpadScreen *pLpad = dynamic_cast<NetLpadScreen *>(TheUI.mCurrentScreen);
        pLpad->SetFocus(TheUI.FindPanel(pLpad->mButtonPanel, false));
        BootScreen *pBoot = dynamic_cast<BootScreen *>(TheUI.FindScreen(kBootScreen, false));
        pBoot->mPlayerName = player->mPlayer.mName.c_str();
        pBoot->mPlayer = player->mId;
        TheUI.GotoScreen(pBoot);
        return false;
    }
    return nButton == kPadCross;
}

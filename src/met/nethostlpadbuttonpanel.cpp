#include "met/nethostlpadbuttonpanel.h"

#include "game/gamedb.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "ui/uicomponent.h"

namespace {

constexpr char kPanelMeshFormat[] = "%s_panel.mesh";
constexpr char kBgMeshFormat[] = "%s_bg.mesh";
constexpr char kFocusedPanelMat[] = "panel_sub_hi.mat";
constexpr char kPanelMat[] = "panel_sub.mat";
constexpr char kFocusedBgMat[] = "bg_hi.mat";
constexpr char kBgMat[] = "bg_no.mat";
constexpr char kLaunchComponent[] = "launch";
constexpr char kShareComponent[] = "share";
constexpr char kEditComponent[] = "edit";
constexpr char kDataComponent[] = "data";

// The value of NetLaunchpadPlayer::mReady for a ready player.
constexpr int kReady = 1;

// The players a session needs to launch or to share a remix.
constexpr unsigned int kMinPlayers = 2;

// Give a mesh named by a format a material.
void SetMeshMat(const char *pszFormat, const char *pszPanel, const char *pszMat) {
    dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString(pszFormat, pszPanel)))
        ->SetMat(dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(pszMat)));
}

} // namespace

void NetHostLPadButtonPanel::Focus() {
    UIPanel::Focus();
    if (!mLoaded) {
        return;
    }
    SetMeshMat(kPanelMeshFormat, mName, kFocusedPanelMat);
    SetMeshMat(kBgMeshFormat, mName, kFocusedBgMat);
}

void NetHostLPadButtonPanel::Unfocus() {
    SetMeshMat(kPanelMeshFormat, mName, kPanelMat);
    SetMeshMat(kBgMeshFormat, mName, kBgMat);
}

void NetHostLPadButtonPanel::Update(std::list<NetLaunchpadPlayer> *pPlayers) {
    bool bCanLaunch = true;
    for (const auto &player : *pPlayers) {
        if (player.mReady != kReady) {
            bCanLaunch = false;
            break;
        }
    }
    if (pPlayers->size() < kMinPlayers) {
        bCanLaunch = false;
    }

    if (bCanLaunch) {
        UIComponent *pLaunch = FindComponent(kLaunchComponent, false);
        if (TheGameDb->mRemixReadOnly != 0 && TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
            pLaunch->SetState(UIComponent::kStateDisabled, false);
        } else if (pLaunch->GetState() == UIComponent::kStateDisabled) {
            pLaunch->SetState(UIComponent::kStateNormal, false);
        }

        UIComponent *pShare = FindComponent(kShareComponent, false);
        if (TheGameDb->mLoadRemix != 0 && pPlayers->size() >= kMinPlayers) {
            if (pShare->GetState() != UIComponent::kStateSelected) {
                pShare->SetState(UIComponent::kStateNormal, false);
            }
        } else {
            if (pShare->GetState() == UIComponent::kStateSelected) {
                SetFocus(FindComponent(kEditComponent, false), kPadNone);
            }
            pShare->SetState(UIComponent::kStateDisabled, false);
        }
    } else {
        UIComponent *pLaunch = FindComponent(kLaunchComponent, false);
        if (pLaunch == mFocus) {
            SetFocus(FindComponent(kEditComponent, false), kPadNone);
        }
        pLaunch->SetState(UIComponent::kStateDisabled, false);
        FindComponent(kShareComponent, false)->SetState(UIComponent::kStateDisabled, false);
    }

    if (mFocus != nullptr) {
        return;
    }
    if (TheNetLaunchpad != nullptr && TheNetLaunchpad->IsGuest() == 0) {
        SetFocus(FindComponent(kEditComponent, false), kPadNone);
    } else {
        SetFocus(FindComponent(kDataComponent, false), kPadNone);
    }
}

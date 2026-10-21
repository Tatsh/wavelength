#include "met/netmainlaunchpadspanel.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/metagameutil.h"
#include "met/netmainlaunchpadslist.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtext.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uiscreen.h"

namespace {

constexpr char kListComponent[] = "list";
constexpr char kCursorComponent[] = "cursor";
constexpr char kMainPanel[] = "fn_main";
constexpr char kPlayersScreen[] = "fn_main_join";
constexpr char kJoinScreen[] = "net_join_lpad";
constexpr char kHostText[] = "fn_main_join_21.txt";
constexpr char kRankMeshFormat[] = "fn_main_join_rank0%d.mesh";
constexpr char kNameTextFormat[] = "fn_main_join_%d.txt";
constexpr char kCustomNameToken[] = "net_custom_name";
#ifdef VIDEO_STANDARD_PAL
// The token of the text of a launchpad whose song this console lacks.
constexpr char kUnavailable[] = "unavailable";
#else
constexpr char kUnavailable[] = "Unavailable";
#endif
constexpr char kAnyArena[] = "";

// The rows of players the panel shows.
constexpr int kPlayerRows = 4;

// The number of the name text of the first row of players.
constexpr int kFirstNameText = 22;

// The arguments of NetLobby::RequestLaunchpads() for the lobby's list.
constexpr int kLobbyPage = 4;
constexpr int kLobbyMode = 1;
constexpr int kAnyRuleSet = 0;
constexpr int kAnySkillLevel = -1;

// The selection that makes SetLaunchpads() keep the current one.
constexpr int kCurrentSelection = -1;

// The milliseconds after a reply before the launchpads are requested again.
constexpr float kRequestIntervalMs = 20000.0f;

NetMainLaunchpadsList *FindList(UIPanel *pPanel) {
    return static_cast<NetMainLaunchpadsList *>(pPanel->FindComponent(kListComponent, false));
}

RndText *FindText(const char *pszName) {
    return dynamic_cast<RndText *>(TheManager.Find(pszName));
}

RndMesh *FindRankMesh(int nRow) {
    return dynamic_cast<RndMesh *>(TheManager.Find(FormatString(kRankMeshFormat, nRow)));
}

RndText *FindNameText(int nRow) {
    return FindText(FormatString(kNameTextFormat, nRow + kFirstNameText));
}

} // namespace

void NetMainLaunchpadsPanel::RequestUpdate() {
    TheNetLobby->RequestLaunchpads(
        this, kLobbyPage, kLobbyMode, kAnyArena, kAnyRuleSet, kAnySkillLevel);
}

void NetMainLaunchpadsPanel::Focus() {
    if (mLoaded == 0) {
        return;
    }
    FocusChangePanel::Focus();
    mHilite = 1;
    FindList(this)->UpdateCursor();
}

void NetMainLaunchpadsPanel::Unfocus() {
    if (mLoaded == 0) {
        return;
    }
    FocusChangePanel::Unfocus();
    mHilite = 0;
    FindList(this)->SetCursorSelected(false);
    HidePlayers();
}

void NetMainLaunchpadsPanel::HidePlayers() {
    FindText(kHostText)->SetShowing(false);
    for (int nRow = 0; nRow < kPlayerRows; ++nRow) {
        FindRankMesh(nRow + 1)->SetShowing(false);
        FindNameText(nRow)->SetShowing(false);
    }
}

void NetMainLaunchpadsPanel::ShowLaunchpad(NetLaunchpadInfo *pLaunchpad) {
    RndText *pHost = FindText(kHostText);
    pHost->SetShowing(true);
    const NetGameParams &params = pLaunchpad->mParams;
    if (pLaunchpad->mOpen == 0) {
#ifdef VIDEO_STANDARD_PAL
        pHost->SetText(TheLocale.Localize(kUnavailable, true));
#else
        pHost->SetText(kUnavailable);
#endif
    } else if (params.GetLoadRemix() != 0) {
        const String format(TheLocale.Localize(kCustomNameToken, true));
        pHost->SetText(FormatString(format.c_str(), params.GetRemixName().c_str()));
    } else {
        const SongEntry entry{TheGameDb->FindSong(params.GetSong().c_str())};
        pHost->SetText(entry.GetTitle());
    }
    mLaunchpad = pLaunchpad->mLaunchpadId;
}

void NetMainLaunchpadsPanel::ShowPlayers(std::list<LobbyPlayer> *pPlayers, int nLaunchpad) {
    if (nLaunchpad != mLaunchpad) {
        TheNetLobby->RequestLaunchpadPlayers(TheUI.FindScreen(kPlayersScreen, false), mLaunchpad);
        return;
    }
    int nRow = 0;
    for (const auto &player : *pPlayers) {
        RndMesh *pRank = FindRankMesh(nRow + 1);
        pRank->SetShowing(true);
        pRank->SetMat(FindRankMaterial(player.GetRankIcon()));
        RndText *pName = FindNameText(nRow);
        pName->SetShowing(true);
        pName->SetText(player.GetName().c_str());
        ++nRow;
    }
}

bool NetMainLaunchpadsPanel::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nLobbyLaunchpadsMsgType) {
        return HandleLaunchpads(static_cast<LobbyLaunchpadsMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return NetMainPanel::DispatchPriv(pMsg);
}

bool NetMainLaunchpadsPanel::HandleLaunchpads(LobbyLaunchpadsMsg *pMsg) {
    if (mLoaded) {
        NetMainLaunchpadsList *pList = FindList(this);
        pList->SetLaunchpads(pMsg->GetLaunchpads(), kCurrentSelection);
        if (TheUI.GetCurrentScreen()->GetFocusPanel() != this) {
            pList->SetCursorSelected(false);
            HidePlayers();
        }
        HideFlash();
    }
    mRequestTime = SystemMs() + kRequestIntervalMs;
    return false;
}

bool NetMainLaunchpadsPanel::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->GetButton() == kPadCross &&
        std::strcmp(pMsg->GetComponent()->GetName(), kCursorComponent) == 0) {
        TheUI.GetCurrentScreen()->SetFocus(TheUI.FindPanel(kMainPanel, false));
        TheUI.GotoScreen(kJoinScreen);
    }
    return false;
}

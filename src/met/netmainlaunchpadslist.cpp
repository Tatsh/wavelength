#include "met/netmainlaunchpadslist.h"

#include <iterator>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/metagameutil.h"
#include "met/netjoinlpadscreen.h"
#include "met/netmainlaunchpadspanel.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/listat.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/rndfont.h"
#include "rnd/rndmanager.h"
#include "ui/uimanager.h"

namespace {

constexpr char kBusyToken[] = "busy";
constexpr char kFullToken[] = "full";
constexpr char kNoPowerupsToken[] = "powerup_na";
constexpr char kUnavailable[] = "Unavailable";
constexpr char kPlayersFormat[] = "%d/%d";
constexpr char kGreyFont[] = "lucida_1_grey.font";
constexpr char kRemixFont[] = "lucida_1_cyan.font";
constexpr char kFont[] = "lucida_1_white.font";
constexpr char kPlayersScreen[] = "fn_main_join";
constexpr char kJoinScreen[] = "net_join_lpad";

// The cells of a row. The European list has no connection column.
enum Column {
    kColumnStatus = 0,
    kColumnMode = 1,
    kColumnDifficulty = 2,
#ifdef VIDEO_STANDARD_PAL
    kColumnPowerups = 3,
    kColumnArtist = 4,
#else
    kColumnConnection = 3,
    kColumnPowerups = 4,
    kColumnArtist = 5,
#endif
};

// The selection SetLaunchpads() passes on to keep the current one.
constexpr int kNoSelection = -1;

} // namespace

void NetMainLaunchpadsList::SetLaunchpads(std::list<NetLaunchpadInfo> *pLaunchpads, int nSelected) {
    mLaunchpads = *pLaunchpads;
    Refresh(static_cast<int>(pLaunchpads->size()),
            nSelected > kNoSelection ? nSelected : kKeepSelection);
}

void NetMainLaunchpadsList::UpdateRow(int nRow, int nItem) {
    const NetLaunchpadInfo &launchpad = *ListAt(mLaunchpads, nItem);
    const NetGameParams &params = launchpad.mParams;
    const bool bGrey = launchpad.mStatus != NetLaunchpadInfo::kStatusOpen || launchpad.mOpen == 0;

    switch (launchpad.mStatus) {
    case NetLaunchpadInfo::kStatusBusy:
        SetCellText(nRow, kColumnStatus, TheLocale.Localize(kBusyToken, true));
        break;
    case NetLaunchpadInfo::kStatusFull:
        SetCellText(nRow, kColumnStatus, TheLocale.Localize(kFullToken, true));
        break;
    case NetLaunchpadInfo::kStatusOpen:
        SetCellText(nRow,
                    kColumnStatus,
                    FormatString(kPlayersFormat, launchpad.mPlayerCount, params.GetMaxPlayers()));
        break;
    default:
        break;
    }

    SetCellText(nRow, kColumnMode, TheGameDb->GetModeName(params.GetRuleSet()));
    if (params.GetRuleSet() == GameDb::kRuleSetDuel) {
        SetCellText(nRow, kColumnDifficulty, GetDuelDifficultyAbbrev(params.GetSkillLevel()));
    } else {
#ifdef VIDEO_STANDARD_PAL
        SetCellText(nRow, kColumnDifficulty, GetSkillAbbrev(params.GetSkillLevel()));
#else
        SetCellText(nRow,
                    kColumnDifficulty,
                    TheGameDb->GetDifficultyName(params.GetSkillLevel(), params.GetRuleSet()));
#endif
    }
#ifndef VIDEO_STANDARD_PAL
    SetCellText(nRow, kColumnConnection, GetConnectionTypeAbbrev(launchpad.mConnectionType));
#endif
    if (params.GetRuleSet() == GameDb::kRuleSetGame) {
        SetCellText(nRow, kColumnPowerups, TheGameDb->GetPowerupName(params.GetPowerupLevel()));
    } else {
        SetCellText(nRow, kColumnPowerups, TheLocale.Localize(kNoPowerupsToken, true));
    }
    if (launchpad.mOpen != 0) {
        const SongEntry entry{TheGameDb->FindSong(params.GetSong().c_str())};
        SetCellText(nRow, kColumnArtist, entry.GetArtistShortest());
    } else {
        SetCellText(nRow, kColumnArtist, kUnavailable);
    }

    const char *pszFont;
    if (bGrey) {
        pszFont = kGreyFont;
    } else if (params.GetLoadRemix() != 0) {
        pszFont = kRemixFont;
    } else {
        pszFont = kFont;
    }
    SetRowFont(nRow, dynamic_cast<RndFont *>(TheManager.Find(pszFont)));
}

void NetMainLaunchpadsList::UpdateCursor() {
    UIList::UpdateCursor();
    auto *pPanel = static_cast<NetMainLaunchpadsPanel *>(TheUI.FindPanel(mPanelName, false));
    pPanel->HidePlayers();
    if (static_cast<unsigned int>(mSelected) < mLaunchpads.size()) {
        NetLaunchpadInfo *pLaunchpad = &*ListAt(mLaunchpads, mSelected);
        pPanel->ShowLaunchpad(pLaunchpad);
        TheNetLobby->RequestLaunchpadPlayers(TheUI.FindScreen(kPlayersScreen, false),
                                             pLaunchpad->mLaunchpadId);
    }
}

bool NetMainLaunchpadsList::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->GetButton() == kPadCross && pMsg->GetPressed() != 0) {
        if (mLaunchpads.empty()) {
            return false;
        }
        const NetLaunchpadInfo &launchpad = *ListAt(mLaunchpads, mSelected);
        if (launchpad.mOpen == 0 || launchpad.mStatus != NetLaunchpadInfo::kStatusOpen) {
            return false;
        }
        dynamic_cast<NetJoinLPadScreen *>(TheUI.FindScreen(kJoinScreen, false))
            ->SetLaunchpad(launchpad.mLaunchpadId, launchpad.mLaunchpadWorld);
    }
    return UIList::HandleJoypad(pMsg);
}

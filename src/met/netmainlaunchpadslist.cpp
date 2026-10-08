#include "met/netmainlaunchpadslist.h"

#include <iterator>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/metagameutil.h"
#include "met/netjoinlpadscreen.h"
#include "met/netmainlaunchpadspanel.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/font.h"
#include "rnd/manager.h"
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

// The cells of a row.
enum Column {
    kColumnStatus = 0,
    kColumnMode = 1,
    kColumnDifficulty = 2,
    kColumnConnection = 3,
    kColumnPowerups = 4,
    kColumnArtist = 5,
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
    const NetLaunchpadInfo &launchpad = *std::next(mLaunchpads.begin(), nItem);
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
                    FormatString(kPlayersFormat, launchpad.mPlayerCount, params.mMaxPlayers));
        break;
    default:
        break;
    }

    SetCellText(nRow, kColumnMode, TheGameDb->GetModeName(params.mRuleSet));
    if (params.mRuleSet == GameDb::kRuleSetDuel) {
        SetCellText(nRow, kColumnDifficulty, GetDuelDifficultyAbbrev(params.mSkillLevel));
    } else {
        SetCellText(nRow,
                    kColumnDifficulty,
                    TheGameDb->GetDifficultyName(params.mSkillLevel, params.mRuleSet));
    }
    SetCellText(nRow, kColumnConnection, GetConnectionTypeAbbrev(launchpad.mConnectionType));
    if (params.mRuleSet == GameDb::kRuleSetGame) {
        SetCellText(nRow, kColumnPowerups, TheGameDb->GetPowerupName(params.mPowerupLevel));
    } else {
        SetCellText(nRow, kColumnPowerups, TheLocale.Localize(kNoPowerupsToken, true));
    }
    if (launchpad.mOpen != 0) {
        const SongEntry entry{TheGameDb->FindSong(params.mSong.c_str())};
        SetCellText(nRow, kColumnArtist, entry.GetArtistShortest());
    } else {
        SetCellText(nRow, kColumnArtist, kUnavailable);
    }

    const char *pszFont;
    if (bGrey) {
        pszFont = kGreyFont;
    } else if (params.mLoadRemix != 0) {
        pszFont = kRemixFont;
    } else {
        pszFont = kFont;
    }
    SetRowFont(nRow, dynamic_cast<Rnd::Font *>(Rnd::TheManager.Find(pszFont)));
}

void NetMainLaunchpadsList::UpdateCursor() {
    UIList::UpdateCursor();
    auto *pPanel = static_cast<NetMainLaunchpadsPanel *>(TheUI.FindPanel(mPanelName, false));
    pPanel->HidePlayers();
    if (static_cast<unsigned int>(mSelected) < mLaunchpads.size()) {
        NetLaunchpadInfo *pLaunchpad = &*std::next(mLaunchpads.begin(), mSelected);
        pPanel->ShowLaunchpad(pLaunchpad);
        TheNetLobby->RequestLaunchpadPlayers(TheUI.FindScreen(kPlayersScreen, false),
                                             pLaunchpad->mLaunchpadId);
    }
}

bool NetMainLaunchpadsList::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mButton == kPadCross && pMsg->mPressed != 0) {
        if (mLaunchpads.empty()) {
            return false;
        }
        const NetLaunchpadInfo &launchpad = *std::next(mLaunchpads.begin(), mSelected);
        if (launchpad.mOpen == 0 || launchpad.mStatus != NetLaunchpadInfo::kStatusOpen) {
            return false;
        }
        dynamic_cast<NetJoinLPadScreen *>(TheUI.FindScreen(kJoinScreen, false))
            ->SetLaunchpad(launchpad.mLaunchpadId, launchpad.mLaunchpadWorld);
    }
    return UIList::HandleJoypad(pMsg);
}

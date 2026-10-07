#include "met/netsorteddatapanel.h"

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/metagameutil.h"
#include "netflow/netlobby.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kRankMeshFormat[] = "fn_sorted_pic_r_0%d.mesh";
constexpr char kNameFormat[] = "0%d";
constexpr char kGenreComponent[] = "genre";
constexpr char kSortedScreen[] = "fn_sorted";
constexpr char kNoText[] = "";

Rnd::Mesh *FindRankMesh(int nRow) {
    return dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString(kRankMeshFormat, nRow)));
}

} // namespace

NetSortedDataPanel::NetSortedDataPanel(DataArray *pData, const char *pszDir)
    : SongPicPanel(pData, pszDir), mLaunchpad(0) {
}

void NetSortedDataPanel::HidePlayers() {
    for (int nRow = 1; nRow <= kNumPlayerRows; ++nRow) {
        FindRankMesh(nRow)->SetShowing(false);
        FindComponent(FormatString(kNameFormat, nRow), false)->SetShowing(false);
    }
}

void NetSortedDataPanel::SetPlayers(std::list<LobbyPlayer> *pPlayers, int nLaunchpad) {
    HidePlayers();
    if (mLaunchpad != nLaunchpad) {
        TheNetLobby->RequestLaunchpadPlayers(TheUI.FindScreen(kSortedScreen, false), mLaunchpad);
        return;
    }
    int nRow = 0;
    for (const auto &player : *pPlayers) {
        ++nRow;
        Rnd::Mesh *pRank = FindRankMesh(nRow);
        pRank->SetShowing(true);
        pRank->SetMat(FindRankMaterial(player.mRankIcon));
        UIComponent *pName = FindComponent(FormatString(kNameFormat, nRow), false);
        pName->SetShowing(true);
        pName->SetText(player.mName.c_str());
    }
}

void NetSortedDataPanel::ShowLaunchpad(NetLaunchpadInfo *pLaunchpad) {
    UIComponent *pGenre = FindComponent(kGenreComponent, false);
    const NetGameParams &params = pLaunchpad->mParams;
    if (pLaunchpad->mOpen != 0) {
        if (params.mLoadRemix != 0) {
            pGenre->SetText(params.mRemixName.c_str());
        } else {
            const SongEntry entry{TheGameDb->FindSong(params.mSong.c_str())};
            pGenre->SetText(entry.GetGenre());
        }
        SetPictureShowing(true);
        SetBandPicture(params.mSong.c_str(), true, false, false);
    } else {
        pGenre->SetText(kNoText);
        SetPictureShowing(false);
    }
    mLaunchpad = pLaunchpad->mLaunchpadId;
}

void NetSortedDataPanel::Reset() {
    HidePlayers();
    FindComponent(kGenreComponent, false)->SetText(kNoText);
    SetPictureShowing(false);
}

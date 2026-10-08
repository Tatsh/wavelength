#include "met/playlist.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/songpicpanel.h"
#include "os/locale.h"
#include "rnd/font.h"
#include "rnd/manager.h"
#include "rnd/mat.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPicPanelTag[] = "pic_panel";
constexpr char kLightOnMat[] = "list_led_on.mat";
constexpr char kLightOffMat[] = "list_led_off.mat";
constexpr char kGroupFont[] = "neuro_1_white.font";
constexpr char kSongFont[] = "lucida_1_white.font";
constexpr char kAllSongsToken[] = "jbox_all_redbook";
constexpr char kAllArena[] = "all";

// The cells of a row.
enum Column {
    kColumnLight = 0,
    kColumnName = 1,
};

// The entry that turns every entry with it.
constexpr int kAllEntry = 0;

Rnd::Mat *FindLightMat(int nSelected) {
    return dynamic_cast<Rnd::Mat *>(
        Rnd::TheManager.Find(nSelected != 0 ? kLightOnMat : kLightOffMat));
}

Rnd::Font *FindFont(const char *pszFont) {
    return dynamic_cast<Rnd::Font *>(Rnd::TheManager.Find(pszFont));
}

} // namespace

PlayList::PlayList(DataArray *pData, const char *pszPanel) : FreqList(pData, pszPanel) {
    pData->FindString(kPicPanelTag, &mPicPanel, false);
}

void PlayList::SetSongs(const std::vector<PlaylistSongData> &songs, int nNumGroups) {
    mSongs = songs;
    mNumGroups = nNumGroups;
    Refresh(static_cast<int>(songs.size()), kKeepSelection);
}

void PlayList::UpdateRow(int nRow, int nItem) {
    SetCellMat(nRow, kColumnLight, FindLightMat(mSongs[nItem].mSelected));
    if (nItem < mNumGroups) {
        SetCellFont(nRow, kColumnName, FindFont(kGroupFont));
        SetCellText(nRow, kColumnName, mSongs[nItem].mSong);
    } else {
        SetCellFont(nRow, kColumnName, FindFont(kSongFont));
        const SongEntry entry{TheGameDb->FindSong(mSongs[nItem].mSong)};
        SetCellText(nRow, kColumnName, entry.GetArtistShort());
    }
}

void PlayList::UpdateCursor() {
    UIList::UpdateCursor();
    auto *pPicture = static_cast<SongPicPanel *>(TheUI.FindPanel(mPicPanel.c_str(), false));
    if (mSelected < mNumGroups) {
        if (std::strcmp(TheLocale.Localize(kAllSongsToken, true), mSongs[mSelected].mSong) == 0) {
            pPicture->SetArenaPicture(kAllArena);
            pPicture->SetPictureShowing(true);
        }
    } else {
        const SongEntry entry{TheGameDb->FindSong(mSongs[mSelected].mSong)};
        pPicture->SetBandPicture(entry.GetName(), true, false, false);
        pPicture->SetPictureShowing(true);
    }
}

void PlayList::ToggleSelected() {
    mSongs[mSelected].mSelected ^= 1;
    if (mSelected == kAllEntry) {
        for (unsigned int i = kAllEntry + 1; i < mSongs.size(); ++i) {
            mSongs[i].mSelected = mSongs[mSelected].mSelected;
        }
        Refresh(static_cast<int>(mSongs.size()), kKeepSelection);
    } else {
        SetCellMat(mCursorRow, kColumnLight, FindLightMat(mSongs[mSelected].mSelected));
    }
}

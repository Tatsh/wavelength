#include "met/songsellist.h"

#include <string.h>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/songpicpanel.h"
#include "os/locale.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

// The choices other than a song, with the arenas whose pictures they show.
constexpr char kRandomToken[] = "host_random";
constexpr char kRandomArena[] = "random";
constexpr char kCustomToken[] = "host_custom";
constexpr char kCustomArena[] = "custom";
constexpr char kAllToken[] = "search_all";
constexpr char kAllArena[] = "all";

} // namespace

SongSelList::SongSelList(DataArray *pData, const char *pszPanel) : FreqList(pData, pszPanel) {
    pData->FindSymbol("pic_panel", &mPicPanel, false);
}

void SongSelList::SetSongs(const std::vector<String> &songs, int nChoiceCount) {
    mSongs = songs;
    mChoiceCount = nChoiceCount;
    Refresh(static_cast<int>(songs.size()), kKeepSelection);
}

void SongSelList::UpdateRow(int nRow, int nItem) {
    if (nItem < mChoiceCount) {
        SetCellText(nRow, 0, mSongs[nItem].c_str());
        return;
    }
    const SongEntry entry{TheGameDb->FindSong(mSongs[nItem].c_str())};
    SetCellText(nRow, 0, entry.GetArtistShort());
}

void SongSelList::UpdateCursor() {
    UIList::UpdateCursor();
    if (mItemCount == 0) {
        return;
    }
    UIComponent *pGenre = TheUI.FindComponent(mPicPanel, "genre", false);
    SongPicPanel *pPicPanel = static_cast<SongPicPanel *>(TheUI.FindPanel(mPicPanel, false));
    if (mSelected >= mChoiceCount) {
        const SongEntry entry{TheGameDb->FindSong(mSongs[mSelected].c_str())};
        pGenre->SetText(entry.GetGenre());
        pPicPanel->SetPictureShowing(true);
        pPicPanel->SetBandPicture(entry.GetName(), true, false, false);
        return;
    }
    if (strcmp(TheLocale.Localize(kRandomToken, true), mSongs[mSelected].c_str()) == 0) {
        pGenre->SetText(TheLocale.Localize(kRandomToken, true));
        pPicPanel->SetPictureShowing(true);
        pPicPanel->SetArenaPicture(kRandomArena);
    } else if (strcmp(TheLocale.Localize(kCustomToken, true), mSongs[mSelected].c_str()) == 0) {
        pGenre->SetText(TheLocale.Localize(kCustomToken, true));
        pPicPanel->SetPictureShowing(true);
        pPicPanel->SetArenaPicture(kCustomArena);
    } else if (strcmp(TheLocale.Localize(kAllToken, true), mSongs[mSelected].c_str()) == 0) {
        pGenre->SetText(TheLocale.Localize(kAllToken, true));
        pPicPanel->SetPictureShowing(true);
        pPicPanel->SetArenaPicture(kAllArena);
    } else {
        pGenre->SetText("");
        pPicPanel->SetPictureShowing(false);
    }
}

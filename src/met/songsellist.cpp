#include "met/songsellist.h"

#include <string.h>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/songpicpanel.h"
#include "os/locale.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

// A choice other than a song, which shows its label as the genre and the picture of an arena.
struct Choice {
    const char *mToken;
    const char *mArena;
};

constexpr Choice kChoices[] = {
    {"host_random", "random"},
    {"host_custom", "custom"},
    {"search_all", "all"},
};

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
    for (const Choice &choice : kChoices) {
        if (strcmp(TheLocale.Localize(choice.mToken, true), mSongs[mSelected].c_str()) == 0) {
            pGenre->SetText(TheLocale.Localize(choice.mToken, true));
            pPicPanel->SetPictureShowing(true);
            pPicPanel->SetArenaPicture(choice.mArena);
            return;
        }
    }
    pGenre->SetText("");
    pPicPanel->SetPictureShowing(false);
}

#include "met/repostoollist.h"

#ifdef VIDEO_STANDARD_PAL

#include "game/gamedb.h"
#include "game/songentry.h"
#include "os/listat.h"
#include "os/string.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"

namespace {

constexpr char kReadOnlyLabel[] = "read_only";
constexpr char kNoteLabel[] = "note";
constexpr char kSongFormat[] = "%s/%s";

constexpr int kNameCell = 0;
constexpr bool kNoPicture = false;

} // namespace

ReposToolList::ReposToolList(DataArray *pData, const char *pszPanel) : FreqList(pData, pszPanel) {
    pData->FindSymbol("pic_panel", &mPicPanel, false);
}

void ReposToolList::SetRemixes(const std::list<NetRepoRemix> &remixes) {
    mRemixes = remixes;
    Refresh(static_cast<int>(remixes.size()), kKeepSelection);
}

void ReposToolList::UpdateRow(int nRow, int nItem) {
    if (static_cast<unsigned int>(nItem) >= mRemixes.size()) {
        return;
    }
    SetCellText(nRow, kNameCell, ListAt(mRemixes, nItem)->mInfo.mName);
}

void ReposToolList::UpdateCursor() {
    UIList::UpdateCursor();
    if (static_cast<unsigned int>(mSelected) >= mRemixes.size()) {
        return;
    }
    const NetRepoRemix &remix = *ListAt(mRemixes, mSelected);
    remix.mInfo.ShowDetails(mPicPanel, kNoPicture);
    SongEntry song{TheGameDb->FindSong(remix.mInfo.mSong)};
    dynamic_cast<UILabel *>(TheUI.FindComponent(mPicPanel, kReadOnlyLabel, false))
        ->SetText(FormatString(kSongFormat, song.GetArtistShortest(), song.GetRemixTitle()));
    dynamic_cast<UILabel *>(TheUI.FindComponent(mPicPanel, kNoteLabel, false))
        ->SetText(remix.mNote.c_str());
}

#endif

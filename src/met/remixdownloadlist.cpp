#include "met/remixdownloadlist.h"

#include <iterator>

#include "game/gamedb.h"
#include "met/songpicpanel.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"

namespace {

constexpr char kNoteLabel[] = "note";

} // namespace

RemixDownloadList::RemixDownloadList(DataArray *pData, const char *pszPanel)
    : FreqList(pData, pszPanel), mSong{nullptr} {
    pData->FindSymbol("pic_panel", &mPicPanel, false);
    mNoteShowing = 1;
}

void RemixDownloadList::SetNoteShowing(int nShowing) {
    mNoteShowing = nShowing;
    TheUI.FindComponent(mPicPanel, kNoteLabel, false)->SetShowing(nShowing);
    SongPicPanel *pPicPanel = static_cast<SongPicPanel *>(TheUI.FindPanel(mPicPanel, false));
    pPicPanel->SetPictureShowing(!nShowing);
    if (pPicPanel->mPictureShowing) {
        pPicPanel->SetBandPicture(mSong.GetName(), true, false, false);
    }
}

void RemixDownloadList::SetRemixes(const std::list<NetRepoRemix> &remixes) {
    mRemixes = remixes;
    Refresh(static_cast<int>(remixes.size()), kKeepSelection);
}

void RemixDownloadList::UpdateRow(int nRow, int nItem) {
    const NetRepoRemix &remix = *std::next(mRemixes.begin(), nItem);
    String date;
    remix.mInfo.mDate.FormatMonthDay(date);
    SetCellText(nRow, 0, date.c_str());
    SetCellText(nRow, 1, remix.mInfo.mName);
}

void RemixDownloadList::UpdateCursor() {
    UIList::UpdateCursor();
    if (mRemixes.empty()) {
        return;
    }
    const NetRepoRemix &remix = *std::next(mRemixes.begin(), mSelected);
    mSong.mData = TheGameDb->FindSong(remix.mInfo.mSong);
    remix.mInfo.ShowDetails(mPicPanel);
    dynamic_cast<UILabel *>(TheUI.FindComponent(mPicPanel, kNoteLabel, false))
        ->SetText(remix.mNote.c_str());
    SetNoteShowing(mNoteShowing);
}

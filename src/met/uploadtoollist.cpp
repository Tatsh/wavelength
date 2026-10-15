#include "met/uploadtoollist.h"

#ifdef VIDEO_STANDARD_PAL

#include "os/joypad.h"
#include "os/listat.h"
#include "os/locale.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kToolPanel[] = "fn_upload_tool";
constexpr char kSortDateButton[] = "sort_date";
constexpr char kInReposText[] = "yes";
constexpr char kNoText[] = "";

// The cells of a row.
enum UploadCell {
    kUploadCellDate = 0,
    kUploadCellSong = 1,
    kUploadCellTitle = 2,
    kUploadCellInRepos = 3,
};

} // namespace

void UploadToolList::SetFiles(const std::list<UploadInfo> &files, int nSelected) {
    mFiles = files;
    Refresh(static_cast<int>(files.size()), nSelected);
}

void UploadToolList::UpdateRow(int nRow, int nItem) {
    if (static_cast<unsigned int>(nItem) >= mFiles.size()) {
        return;
    }
    const UploadInfo &info = *ListAt(mFiles, nItem);
    char szDate[kUploadDateSize] = "";
    char szSong[kUploadPartSize] = "";
    char szTitle[kUploadPartSize] = "";
    SplitUploadName(info.mName.c_str(), szDate, szSong, szTitle);
    SetCellText(nRow, kUploadCellDate, szDate);
    SetCellText(nRow, kUploadCellSong, szSong);
    SetCellText(nRow, kUploadCellTitle, szTitle);
    if (info.mInRepos != 0) {
        SetCellText(nRow, kUploadCellInRepos, TheLocale.Localize(kInReposText, true));
    } else {
        SetCellText(nRow, kUploadCellInRepos, kNoText);
    }
}

void UploadToolList::UpdateCursor() {
    UIList::UpdateCursor();
}

void UploadToolList::ScrollUp() {
    if (mCursorRow != 0 || mSelected != 0) {
        FreqList::ScrollUp();
        return;
    }
    UIPanel *pPanel = TheUI.FindPanel(kToolPanel, false);
    pPanel->SetFocus(pPanel->FindComponent(kSortDateButton, false), kPadNone);
    SetCursorSelected(false);
}

#endif

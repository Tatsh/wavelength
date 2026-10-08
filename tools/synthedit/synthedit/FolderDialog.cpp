#include "synthedit/FolderDialog.h"

namespace {

// The longest path the status text shows whole.
const int kMaxStatusPathLength = 35;

} // namespace

CFolderDialog::CFolderDialog(LPCTSTR initialFolder, UINT flags, CWnd *parent) {
    if (initialFolder == NULL) {
        mInitialFolder = "";
    } else {
        mInitialFolder = initialFolder;
    }
    memset(&mBrowseInfo, 0, sizeof(mBrowseInfo));
    if (parent == NULL) {
        mBrowseInfo.hwndOwner = NULL;
    } else {
        mBrowseInfo.hwndOwner = parent->GetSafeHwnd();
    }
    mBrowseInfo.pidlRoot = NULL;
    mBrowseInfo.pszDisplayName = mDisplayName;
    mBrowseInfo.lpszTitle = "Current Selection";
    mBrowseInfo.ulFlags = flags | BIF_STATUSTEXT;
    mBrowseInfo.lpfn = BrowseCallbackProc;
    mBrowseInfo.lParam = reinterpret_cast<LPARAM>(this);
}

int CALLBACK CFolderDialog::BrowseCallbackProc(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData) {
    CFolderDialog *dialog = reinterpret_cast<CFolderDialog *>(lpData);
    dialog->OnBrowseMessage(hwnd, msg, lParam);
    return 0;
}

CFolderDialog::~CFolderDialog() {
}

void CFolderDialog::OnBrowseMessage(HWND hwnd, UINT msg, LPARAM lParam) {
    mWnd = hwnd;
    switch (msg) {
    case BFFM_INITIALIZED:
        OnInitialized();
        break;
    case BFFM_SELCHANGED:
        OnSelChanged(reinterpret_cast<LPITEMIDLIST>(lParam));
        break;
    }
}

int CFolderDialog::DoModal() {
    int result = IDOK;
    mSelectedFolder = mInitialFolder;
    LPITEMIDLIST pidl = NULL;
    pidl = SHBrowseForFolder(&mBrowseInfo);
    if (pidl != NULL && SHGetPathFromIDList(pidl, mPath)) {
        mSelectedFolder = mPath;
        result = IDOK;
    } else {
        result = IDCANCEL;
    }
    if (pidl != NULL) {
        LPMALLOC shellMalloc;
        SHGetMalloc(&shellMalloc);
        shellMalloc->Free(pidl);
        shellMalloc->Release();
    }
    return result;
}

CString CFolderDialog::GetSelectedFolder() {
    return mSelectedFolder;
}

void CFolderDialog::EnableOK(BOOL enable) {
    SendMessage(mWnd, BFFM_ENABLEOK, 0, enable != FALSE);
}

void CFolderDialog::SetSelection(LPCTSTR path) {
    SendMessage(mWnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(path));
}

void CFolderDialog::SetSelection(LPCITEMIDLIST pidl) {
    SendMessage(mWnd, BFFM_SETSELECTION, FALSE, reinterpret_cast<LPARAM>(pidl));
}

void CFolderDialog::SetStatusText(LPCTSTR text) {
    SendMessage(mWnd, BFFM_SETSTATUSTEXT, 0, reinterpret_cast<LPARAM>(text));
}

CString CFolderDialog::ShortenPath(const CString &path) {
    CString shortPath;
    if (path.GetLength() <= kMaxStatusPathLength) {
        shortPath = path;
    } else {
        shortPath = path.Left(kMaxStatusPathLength) + "...";
    }
    return shortPath;
}

void CFolderDialog::OnInitialized() {
    SetSelection(static_cast<LPCTSTR>(mInitialFolder));
    SetStatusText(ShortenPath(mInitialFolder));
}

void CFolderDialog::OnSelChanged(LPITEMIDLIST pidl) {
    SHGetPathFromIDList(pidl, mPath);
    mSelectedFolder = mPath;
    SetStatusText(ShortenPath(mSelectedFolder));
}

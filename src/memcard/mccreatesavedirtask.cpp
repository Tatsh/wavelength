#include "memcard/mccreatesavedirtask.h"

#include <string.h>

#include <libmc.h>

#include "memcard/memcardconfig.h"
#include "memcard/shiftjis.h"

namespace {

// Bytes of the title once converted to Shift-JIS, the terminator included.
constexpr int kTitleBufferSize = 64;

} // namespace

MCCreateSaveDirTask::MCCreateSaveDirTask() {
}

void MCCreateSaveDirTask::Set(int nPort, const char *pszTitle) {
    SetPort(nPort);
    char szTitle[kTitleBufferSize];
    AsciiToShiftJis(pszTitle, szTitle);
    memcpy(g_MemcardIconSys.TitleName, szTitle, strlen(szTitle) + 1);
}

void MCCreateSaveDirTask::OnStart() {
    if (g_bMemcardDirExists != 0) {
        Finish(true);
        return;
    }
    mFiles.clear();

    const char *pszIcon;
    g_pMemcardConfig->FindSymbol("icon_file", &pszIcon, true);
    FileData icon = {String(FormatString("%s/%s", g_MemcardPath.c_str(), pszIcon)),
                     g_pMemcardIcon,
                     g_nMemcardIconSize};
    mFiles.push_back(icon);

    FileData iconSys = {String(FormatString("%s/icon.sys", g_MemcardPath.c_str())),
                        &g_MemcardIconSys,
                        sizeof(sceMcIconSys)};
    mFiles.push_back(iconSys);

    // Yes, the binary saves two bytes of the icon pointer as the file named after the directory.
    FileData dirFile = {String(FormatString("%s/%s", g_MemcardPath.c_str(), g_MemcardPath.c_str())),
                        &g_pMemcardIcon,
                        kMemcardDirFileSize};
    mFiles.push_back(dirFile);

    mMkDir.Set(mPort, g_MemcardPath.c_str());
    mMkDir.Start();
}

void MCCreateSaveDirTask::SaveNextFile() {
    if (mFileIndex != static_cast<int>(mFiles.size())) {
        mSaveFile.Stop();
        const FileData &file = mFiles[mFileIndex];
        mSaveFile.Set(mPort, file.mName.c_str(), file.mData, file.mSize);
        mSaveFile.Start();
    } else {
        Finish(true);
    }
}

void MCCreateSaveDirTask::OnPoll() {
    MemcardTask::OnPoll();
    int nState = mMkDir.Poll();
    if (nState == kStateDone || nState == kStateFailed) {
        mFileIndex = 0;
        SaveNextFile();
        mMkDir.Stop();
    } else if (nState == kStateIdle) {
        nState = mSaveFile.Poll();
        if (nState == kStateDone) {
            ++mFileIndex;
            SaveNextFile();
        } else if (nState == kStateFailed) {
            Finish(false);
        }
    }
}

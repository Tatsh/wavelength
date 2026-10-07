#include "memcard/mcsavefreqtask.h"

#include <ctype.h>
#include <strings.h>

#include "memcard/memcardconfig.h"
#include "memcard/memcardtask.h"
#include "os/bufstream.h"

namespace {

// A file that never exists. The delete step removes it when the name did not change, and counts
// the missing file as deleted.
constexpr char kNoPreviousFile[] = "poo.poopoopoo";

} // namespace

MCSaveFreqTask::MCSaveFreqTask() {
    mGetInfo = new MCGetInfoTask;
    mDelete = new MCDeleteTask;
    mCheck = new MCCheckSaveSpaceNeededTask;
    mCreateDir = new MCCreateSaveDirTask;
    mSaveFile = new MCSaveFileTask;
    Add(mGetInfo);
    Add(mDelete);
    Add(mCheck);
    Add(mCreateDir);
    Add(mSaveFile);
}

void MCSaveFreqTask::Set(int nPort,
                         PlayerProfile *pProfile,
                         const char *pszOldName,
                         int nOverwrite) {
    mNeeded = 0;
    mPort = nPort;
    mName = pProfile->mName.c_str();
    mGetInfo->Set(nullptr, nPort, 0);
    for (char *pChar = mName.mBuffer; pChar < mName.mBuffer + mName.mLength; ++pChar) {
        *pChar = tolower(*pChar);
    }
    if (strcasecmp(mName.c_str(), pszOldName) != 0) {
        const char *pszFreqExt;
        g_pMemcardConfig->FindSymbol("freq_ext", &pszFreqExt, true);
        mDelete->Set(mPort, FormatString("%s/%s%s", g_MemcardPath.c_str(), pszOldName, pszFreqExt));
    } else {
        mDelete->Set(mPort, kNoPreviousFile);
    }
    BufStream stream(g_pMemcardBuffer, g_nMemcardBufferSize, true);
    stream << *pProfile;
    const char *pszFreqExt;
    g_pMemcardConfig->FindSymbol("freq_ext", &pszFreqExt, true);
    int nMaxFreqs;
    g_pMemcardConfig->FindInt("max_freqs", &nMaxFreqs, true);
    g_pMemcardConfig->FindInt("max_freq_size", &mFileSize, true);
    mCheck->Set(this, nPort, mName.c_str(), pszFreqExt, nMaxFreqs, mFileSize, nOverwrite);
}

int MCSaveFreqTask::GetNeeded() {
    return mNeeded;
}

void MCSaveFreqTask::OnSaveSpace(int nNeeded) {
    if (MemcardTask::sStatus == MemcardTask::kStatusOk) {
        const char *pszLabel;
        g_pMemcardConfig->FindSymbol("freq_label", &pszLabel, true);
        mCreateDir->Set(mPort, pszLabel);
        const char *pszFreqExt;
        g_pMemcardConfig->FindSymbol("freq_ext", &pszFreqExt, true);
        mSaveFile->Set(mPort,
                       FormatString("%s/%s%s", g_MemcardPath.c_str(), mName.c_str(), pszFreqExt),
                       g_pMemcardBuffer,
                       mFileSize);
    } else {
        mNeeded = nNeeded;
        mCheck->Finish(false);
    }
}

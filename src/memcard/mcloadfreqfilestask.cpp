#include "memcard/mcloadfreqfilestask.h"

#include <algorithm>

#include "memcard/memcardconfig.h"
#include "os/bufstream.h"

void MCLoadFreqFilesTask::Set(int nPort, std::vector<Campaign> *pProfiles) {
    mProfiles = pProfiles;
    mCardPort = nPort;
}

void MCLoadFreqFilesTask::OnStart() {
    std::vector<Campaign> *pProfiles = mProfiles;
    const int nCount = sDirCount;
    Campaign blank;
    pProfiles->resize(nCount, blank);
    std::sort(sDirEntries, sDirEntries + sDirCount);
    mIndex = sDirCount - 1;
    LoadNext();
}

void MCLoadFreqFilesTask::LoadNext() {
    if (mIndex >= 0) {
        const char *pszDirExt;
        g_pMemcardConfig->FindSymbol("freq_dir_ext", &pszDirExt, true);
        mLoad.Set(
            nullptr,
            mCardPort,
            FormatString("%s%s/%s", g_pszMemcardBaseDir, pszDirExt, sDirEntries[mIndex].mName),
            g_pMemcardBuffer,
            g_nMemcardBufferSize);
        mLoad.Start();
    } else {
        Finish(true);
    }
}

void MCLoadFreqFilesTask::OnPoll() {
    MemcardTask::OnPoll();
    const int nState = mLoad.Poll();
    if (nState == kStateDone) {
        BufStream stream(g_pMemcardBuffer, g_nMemcardBufferSize, true);
        stream >> mProfiles->at(sDirCount - (mIndex + 1));
        mProfiles->at(sDirCount - (mIndex + 1)).mCustom = 1;
        --mIndex;
        LoadNext();
    } else if (nState == kStateFailed) {
        Finish(false);
    }
}

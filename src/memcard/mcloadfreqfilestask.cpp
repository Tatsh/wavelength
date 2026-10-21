#include "memcard/mcloadfreqfilestask.h"

#include <algorithm>

#include "memcard/memcardconfig.h"
#include "os/bufstream.h"
#ifdef VIDEO_STANDARD_PAL
#include "os/memcardsync.h"
#endif

void MCLoadFreqFilesTask::Set(int nPort, std::vector<Campaign> *pProfiles) {
    mProfiles = pProfiles;
    mCardPort = nPort;
}

void MCLoadFreqFilesTask::OnStart() {
#ifdef VIDEO_STANDARD_PAL
    mProfiles->clear();
    mProfiles->reserve(sDirCount);
    std::sort(sDirEntries, sDirEntries + sDirCount);
    mIndex = 0;
#else
    std::vector<Campaign> *pProfiles = mProfiles;
    const int nCount = sDirCount;
    Campaign blank;
    pProfiles->resize(nCount, blank);
    std::sort(sDirEntries, sDirEntries + sDirCount);
    mIndex = sDirCount - 1;
#endif
    LoadNext();
}

void MCLoadFreqFilesTask::LoadNext() {
#ifdef VIDEO_STANDARD_PAL
    if (mIndex < sDirCount) {
#else
    if (mIndex >= 0) {
#endif
        const char *pszDirExt;
        g_pMemcardConfig->FindSymbol("freq_dir_ext", &pszDirExt, true);
        mLoad.Set(
            nullptr,
            mCardPort,
            FormatString("%s%s/%s", g_pszMemcardBaseDir, pszDirExt, sDirEntries[mIndex].mName),
            g_pMemcardBuffer.get(),
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
#ifdef VIDEO_STANDARD_PAL
        int nMaxSize;
        g_pMemcardConfig->FindInt("max_freq_size", &nMaxSize, true);
        // A file of another size than a whole profile is deleted as corrupt.
        if (MemcardReadSizeIs(nMaxSize)) {
            BufStream stream(g_pMemcardBuffer.get(), g_nMemcardBufferSize, true);
            mProfiles->push_back(Campaign());
            stream >> mProfiles->back();
            mProfiles->back().mCustom = 1;
        } else {
            MemcardDeleteAndWait(mCardPort, g_MemcardOpenPath.c_str());
        }
        ++mIndex;
#else
        BufStream stream(g_pMemcardBuffer.get(), g_nMemcardBufferSize, true);
        stream >> mProfiles->at(sDirCount - (mIndex + 1));
        mProfiles->at(sDirCount - (mIndex + 1)).mCustom = 1;
        --mIndex;
#endif
        LoadNext();
    } else if (nState == kStateFailed) {
        Finish(false);
    }
}

#include "memcard/mcfindremixtask.h"

#include "memcard/memcardconfig.h"

void MCFindRemixTask::Set(int nPort, const char *pszName) {
    SetPort(nPort);
    mName = pszName;
}

void MCFindRemixTask::OnStart() {
    g_RemixPath = "";
    mDirIndex = 0;
    SearchNextDir();
}

void MCFindRemixTask::SearchNextDir() {
    if (mDirIndex < g_nMaxRemixDirs) {
        String path;
        path.Printf("%s/%s", GetRemixDirName(mDirIndex), mName.c_str());
        mGetDir.Set(nullptr, mPort, path.c_str(), g_nMaxRemixesPerDir, 0);
        mGetDir.Start();
    } else {
        sStatus = kStatusNotFound;
        Finish(false);
    }
}

void MCFindRemixTask::OnPoll() {
    MemcardTask::OnPoll();
    const int nState = mGetDir.Poll();
    if (nState != kStateDone && nState != kStateFailed) {
        return;
    }
    if (sDirCount == 0) {
        ++mDirIndex;
        SearchNextDir();
    } else {
        g_RemixPath = "";
        g_RemixPath.Printf("%s/%s", GetRemixDirName(mDirIndex), mName.c_str());
        Finish(true);
    }
}

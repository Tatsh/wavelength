#include "memcard/mcgetremixnamestask.h"

#include "memcard/memcardconfig.h"
#include "os/string.h"

void MCGetRemixNamesTask::Set(int nPort) {
    SetPort(nPort);
}

void MCGetRemixNamesTask::OnStart() {
    mDirIndex = 0;
    g_RemixNames.clear();
    ListNextDir();
}

void MCGetRemixNamesTask::ListNextDir() {
    if (mDirIndex < g_nMaxRemixDirs) {
        String pattern;
        pattern.Printf("%s/*%s", GetRemixDirName(mDirIndex), g_pszRemixExt);
        mGetDir.Set(nullptr, mPort, pattern.c_str(), g_nMaxRemixesPerDir, 0);
        mGetDir.Start();
    } else {
        Finish(true);
    }
}

void MCGetRemixNamesTask::OnPoll() {
    MemcardTask::OnPoll();
    const int nState = mGetDir.Poll();
    if (nState != kStateDone && nState != kStateFailed) {
        return;
    }
    for (int i = 0; i < sDirCount; ++i) {
        String path;
        path.Printf("%s/%s", GetRemixDirName(mDirIndex), sDirEntries[i].mName);
        g_RemixNames.push_back(path);
    }
    ++mDirIndex;
    ListNextDir();
}

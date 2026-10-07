#include "memcard/mcsaveremixtask.h"

#include "memcard/memcardconfig.h"
#include "memcard/memcardtask.h"
#include "os/bufstream.h"

MCSaveRemixTask::MCSaveRemixTask() {
    mCheck = new MCCheckRemixSpaceNeededTask;
    mCreateDir = new MCCreateSaveDirTask;
    mSaveFile = new MCSaveFileTask;
    Add(mCheck);
    Add(mCreateDir);
    Add(mSaveFile);
}

void MCSaveRemixTask::Set(int nPort,
                          const char *pszName,
                          RemixInfo *pInfo,
                          const void *pData,
                          int nSize,
                          int nOverwrite) {
    mPort = nPort;
    mNeeded = 0;
    mName = pszName;
    mCheck->Set(this, nPort, pszName, nSize + static_cast<int>(sizeof(RemixInfo)), nOverwrite);
    BufStream stream(g_pMemcardBuffer, g_nMemcardBufferSize, true);
    stream << *pInfo;
    stream.Write(pData, nSize);
    mSize = stream.Tell();
}

int MCSaveRemixTask::GetNeeded() {
    return mNeeded;
}

void MCSaveRemixTask::OnSaveSpace(int nNeeded) {
    if (MemcardTask::sStatus == MemcardTask::kStatusOk) {
        const char cDirTag = g_MemcardPath[g_MemcardPath.mLength - 1];
        const char *pszLabel;
        g_pMemcardConfig->FindSymbol("remix_label", &pszLabel, true);
        mCreateDir->Set(mPort, FormatString("%s %c", pszLabel, cDirTag));
        const char *pszRemixExt;
        g_pMemcardConfig->FindSymbol("remix_ext", &pszRemixExt, true);
        mSaveFile->Set(mPort,
                       FormatString("%s/%s%s", g_MemcardPath.c_str(), mName.c_str(), pszRemixExt),
                       g_pMemcardBuffer,
                       mSize);
    } else {
        mNeeded = nNeeded;
        mCheck->Finish(false);
    }
}

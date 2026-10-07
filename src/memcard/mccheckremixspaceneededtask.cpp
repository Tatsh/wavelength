#include "memcard/mccheckremixspaceneededtask.h"

#include <string.h>

#include "memcard/memcardconfig.h"
#include "memcard/memcardtask.h"

MCCheckRemixSpaceNeededTask::MCCheckRemixSpaceNeededTask() {
    mGetInfo = new MCGetInfoTask;
    Add(mGetInfo);
    mGetDirs.resize(g_nMaxRemixDirs);
    for (int i = 0; i < g_nMaxRemixDirs; ++i) {
        mGetDirs[i] = new MCGetDirTask;
        Add(mGetDirs[i]);
    }
}

void MCCheckRemixSpaceNeededTask::Set(
    SaveSpaceUser *pUser, int nPort, const char *pszName, int nFileSize, int nOverwrite) {
    mUser = pUser;
    mNeeded = 0;
    mFileName = pszName;
    mOverwrite = nOverwrite;
    mFileSize = nFileSize;
    mRemixCount = 0;
    mDirIndex = 0;
    mFound = 0;
    mFoundSize = 0;
    g_MemcardPath = "";
    g_bMemcardDirExists = 0;
    mGetInfo->Set(this, nPort, 0);
    mFileName += g_pszRemixExt;
    for (unsigned int i = 0; i < mGetDirs.size(); ++i) {
        String pattern;
        pattern.Printf("%s/*%s", GetRemixDirName(i), g_pszRemixExt);
        mGetDirs[i]->Set(this, nPort, pattern.c_str(), g_nMaxRemixesPerDir, 0);
    }
}

void MCCheckRemixSpaceNeededTask::OnDirListed() {
    if (mFound == 0) {
        if (MemcardTask::sDirCount == 0) {
            if (g_MemcardPath.mLength == 0) {
                g_MemcardPath = GetRemixDirName(mDirIndex);
                g_bMemcardDirExists = 0;
            }
        } else {
            int i = 0;
            for (; i < MemcardTask::sDirCount; ++i) {
                if (strcmp(MemcardTask::sDirEntries[i].mName, mFileName.c_str()) == 0) {
                    mFound = 1;
                    mFoundSize = MemcardTask::sDirEntries[i].mSize;
                    g_MemcardPath = GetRemixDirName(mDirIndex);
                    g_bMemcardDirExists = 1;
                    break;
                }
            }
            if (i == MemcardTask::sDirCount) {
                mRemixCount += static_cast<unsigned char>(MemcardTask::sDirCount);
                if (MemcardTask::sDirCount != g_nMaxRemixesPerDir && g_MemcardPath.mLength == 0) {
                    g_MemcardPath = GetRemixDirName(mDirIndex);
                    g_bMemcardDirExists = 1;
                }
            }
        }
    }
    ++mDirIndex;
    if (mDirIndex != mGetDirs.size()) {
        return;
    }
    MemcardTask::sStatus = MemcardTask::kStatusOk;
    mNeeded = BytesToKilobytes(mFileSize - mFoundSize);
    if (mFound == 0) {
        if (mRemixCount < g_nMaxRemixes) {
            if (g_bMemcardDirExists == 0) {
                mNeeded += g_nMemcardDirKilobytes;
            }
        } else {
            MemcardTask::sStatus = MemcardTask::kStatusLimit;
        }
    } else if (mOverwrite == 0) {
        MemcardTask::sStatus = MemcardTask::kStatusExists;
    }
    if (mGetInfo->mFree < mNeeded) {
        MemcardTask::sStatus = MemcardTask::kStatusFull;
    }
}

void MCCheckRemixSpaceNeededTask::OnStop() {
    Finish(MemcardTask::sStatus == MemcardTask::kStatusOk);
    mUser->OnSaveSpace(mNeeded);
}

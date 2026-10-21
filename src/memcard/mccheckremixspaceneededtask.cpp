#include "memcard/mccheckremixspaceneededtask.h"

#include <string.h>

#include "memcard/memcardconfig.h"
#include "memcard/memcardtask.h"

#ifdef VIDEO_STANDARD_PAL
namespace {

// The value of MCCheckRemixSpaceNeededTask::mDirRemixCounts for a directory that does not exist.
constexpr int kDirMissing = -1;

// The kilobytes the directory entry of a new file needs.
constexpr int kFileEntryKilobytes = 1;

} // namespace
#endif

MCCheckRemixSpaceNeededTask::MCCheckRemixSpaceNeededTask() {
    mGetInfo = new MCGetInfoTask;
    Add(mGetInfo);
#ifdef VIDEO_STANDARD_PAL
    mCheckDirs.resize(g_nMaxRemixDirs);
    for (int i = 0; i < g_nMaxRemixDirs; ++i) {
        mCheckDirs[i] = new MCCheckDirExistsTask;
        Add(mCheckDirs[i]);
    }
#endif
    mGetDirs.resize(g_nMaxRemixDirs);
    for (int i = 0; i < g_nMaxRemixDirs; ++i) {
        mGetDirs[i] = new MCGetDirTask;
        Add(mGetDirs[i]);
    }
#ifdef VIDEO_STANDARD_PAL
    mDirRemixCounts.resize(g_nMaxRemixDirs, 0);
#endif
}

void MCCheckRemixSpaceNeededTask::Set(
    SaveSpaceUser *pUser, int nPort, const char *pszName, int nFileSize, int nOverwrite) {
    mUser = pUser;
    mNeeded = 0;
    mFileName = pszName;
#ifdef VIDEO_STANDARD_PAL
    mFileName += g_pszRemixExt;
    mFileSize = nFileSize;
    mOverwrite = nOverwrite;
    mDirIndex = 0;
    mFound = 0;
    mFoundSize = 0;
    g_MemcardPath.Clear();
    g_bMemcardDirExists = 0;
    mGetInfo->Set(this, nPort, 0);
    for (unsigned int i = 0; i < mCheckDirs.size(); ++i) {
        mDirRemixCounts[i] = kDirMissing;
        mCheckDirs[i]->Set(nPort, GetRemixDirName(i), this);
    }
#else
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
#endif
    for (unsigned int i = 0; i < mGetDirs.size(); ++i) {
        String pattern;
        pattern.Printf("%s/*%s", GetRemixDirName(i), g_pszRemixExt);
        mGetDirs[i]->Set(this, nPort, pattern.c_str(), g_nMaxRemixesPerDir, 0);
    }
}

#ifdef VIDEO_STANDARD_PAL
void MCCheckRemixSpaceNeededTask::OnDirChecked(int bWhole) {
    if (bWhole) {
        mDirRemixCounts[mDirIndex] = 0;
    }
    ++mDirIndex;
    if (mDirIndex == mCheckDirs.size()) {
        mDirIndex = 0;
    }
}

void MCCheckRemixSpaceNeededTask::OnDirListed() {
    int &nRemixCount = mDirRemixCounts[mDirIndex];
    if (nRemixCount != kDirMissing) {
        nRemixCount = MemcardTask::sDirCount;
    }
    if (mFound == 0) {
        for (int i = 0; i < MemcardTask::sDirCount; ++i) {
            if (mFileName == MemcardTask::sDirEntries[i].mName) {
                mFound = 1;
                mFoundSize = MemcardTask::sDirEntries[i].mSize;
                g_MemcardPath = GetRemixDirName(mDirIndex);
                g_bMemcardDirExists = 1;
                break;
            }
        }
    }
    ++mDirIndex;
    if (mDirIndex == mCheckDirs.size()) {
        Evaluate();
    }
}

void MCCheckRemixSpaceNeededTask::Evaluate() {
    if (g_MemcardPath.mLength == 0) {
        for (unsigned int i = 0; i < mDirRemixCounts.size(); ++i) {
            const int nRemixCount = mDirRemixCounts[i];
            if (nRemixCount != kDirMissing && nRemixCount < g_nMaxRemixesPerDir) {
                g_MemcardPath = GetRemixDirName(i);
                g_bMemcardDirExists = 1;
                break;
            }
        }
    }
    if (g_MemcardPath.mLength == 0) {
        for (unsigned int i = 0; i < mDirRemixCounts.size(); ++i) {
            if (mDirRemixCounts[i] == kDirMissing) {
                g_MemcardPath = GetRemixDirName(i);
                g_bMemcardDirExists = 0;
                break;
            }
        }
    }
    mNeeded = BytesToKilobytes(mFileSize);
    if (mFound == 0) {
        mNeeded += kFileEntryKilobytes;
    } else {
        mNeeded -= BytesToKilobytes(mFoundSize);
    }
    if (g_bMemcardDirExists == 0) {
        mNeeded += g_nMemcardDirKilobytes;
    }
    if (mFound != 0 && mOverwrite == 0) {
        MemcardTask::sStatus = MemcardTask::kStatusExists;
    } else if (g_MemcardPath.mLength == 0) {
        MemcardTask::sStatus = MemcardTask::kStatusLimit;
    } else if (mGetInfo->GetFree() < mNeeded) {
        MemcardTask::sStatus = MemcardTask::kStatusFull;
    } else {
        MemcardTask::sStatus = MemcardTask::kStatusOk;
    }
}
#else
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
    if (mGetInfo->GetFree() < mNeeded) {
        MemcardTask::sStatus = MemcardTask::kStatusFull;
    }
}
#endif

void MCCheckRemixSpaceNeededTask::OnStop() {
    Finish(MemcardTask::sStatus == MemcardTask::kStatusOk);
    mUser->OnSaveSpace(mNeeded);
}

#include "memcard/mcchecksavespaceneededtask.h"

#include <string.h>

#include "memcard/memcardconfig.h"
#include "memcard/memcardtask.h"

namespace {

// The entries one listing requests.
constexpr int kListingEntryCount = 20;

// Values of MCCheckSaveSpaceNeededTask::mListing.
constexpr int kListingDirectory = 0;
constexpr int kListingFiles = 1;

} // namespace

MCCheckSaveSpaceNeededTask::MCCheckSaveSpaceNeededTask() {
    mGetInfo = new MCGetInfoTask;
    Add(mGetInfo);
    for (int i = 0; i < kSaveSpaceListingCount; ++i) {
        mGetDirs[i] = new MCGetDirTask;
        Add(mGetDirs[i]);
    }
}

void MCCheckSaveSpaceNeededTask::Set(SaveSpaceUser *pUser,
                                     int nPort,
                                     const char *pszName,
                                     const char *pszExt,
                                     int nMaxFiles,
                                     int nFileSize,
                                     int nOverwrite) {
    mUser = pUser;
    mNeeded = 0;
    mFileName = pszName;
    mFileName += pszExt;
    mFileSize = nFileSize;
    mOverwrite = nOverwrite;
    mMaxFiles = nMaxFiles;
    mListing = kListingDirectory;
    mGetInfo->Set(this, nPort, 0);
    const char *pszDirExt;
    g_pMemcardConfig->FindSymbol("freq_dir_ext", &pszDirExt, true);
    g_MemcardPath = FormatString("%s%s", g_pszMemcardBaseDir, pszDirExt);
    g_bMemcardDirExists = 0;
    mGetDirs[kListingDirectory]->Set(this, nPort, g_MemcardPath.c_str(), kListingEntryCount, 0);
    mGetDirs[kListingFiles]->Set(
        this, nPort, FormatString("%s/*%s", g_MemcardPath.c_str(), pszExt), kListingEntryCount, 0);
}

void MCCheckSaveSpaceNeededTask::OnDirListed() {
    if (mListing == kListingDirectory) {
        g_bMemcardDirExists = MemcardTask::sDirCount != 0 ? 1 : 0;
        ++mListing;
        return;
    }
    if (mListing != kListingFiles) {
        return;
    }
    const char *pszName = mFileName.c_str();
    bool bFound = false;
    int nFileCount = 0;
    int nOldSize = 0;
    int i = 0;
    for (; i < MemcardTask::sDirCount; ++i) {
        if (strcmp(MemcardTask::sDirEntries[i].mName, pszName) == 0) {
            bFound = true;
            nOldSize = MemcardTask::sDirEntries[i].mSize;
            break;
        }
    }
    if (i == MemcardTask::sDirCount) {
        nFileCount = i;
        ++mNeeded;
    }
    MemcardTask::sStatus = MemcardTask::kStatusOk;
    mNeeded += BytesToKilobytes(mFileSize - nOldSize);
    if (!bFound) {
        if (nFileCount == mMaxFiles) {
            MemcardTask::sStatus = MemcardTask::kStatusLimit;
        } else if (g_bMemcardDirExists == 0) {
            mNeeded += g_nMemcardDirKilobytes;
        }
    } else if (mOverwrite == 0) {
        MemcardTask::sStatus = MemcardTask::kStatusExists;
    }
    if (mGetInfo->mFree < mNeeded) {
        MemcardTask::sStatus = MemcardTask::kStatusFull;
    }
}

void MCCheckSaveSpaceNeededTask::OnStop() {
    Finish(MemcardTask::sStatus == MemcardTask::kStatusOk);
    mUser->OnSaveSpace(mNeeded);
}

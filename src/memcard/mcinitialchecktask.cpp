#include "memcard/mcinitialchecktask.h"

#include "memcard/memcardconfig.h"
#include "memcard/memcardtask.h"

namespace {

// The entries one listing requests.
constexpr int kListingEntryCount = 20;

// Values of MCInitialCheckTask::mListing.
constexpr int kListingDirectory = 0;
constexpr int kListingFreqs = 1;

} // namespace

MCInitialCheckTask::MCInitialCheckTask() {
    mGetInfo = new MCGetInfoTask;
    Add(mGetInfo);
    for (int i = 0; i < kInitialCheckListingCount; ++i) {
        mGetDirs[i] = new MCGetDirTask;
        Add(mGetDirs[i]);
    }
}

void MCInitialCheckTask::Set(int nPort) {
    mNeeded = 0;
    mListing = kListingDirectory;
    mGetInfo->Set(this, nPort, 1);
    const char *pszDirExt;
    g_pMemcardConfig->FindSymbol("freq_dir_ext", &pszDirExt, true);
    g_MemcardPath = FormatString("%s%s", g_pszMemcardBaseDir, pszDirExt);
    g_bMemcardDirExists = 0;
    mGetDirs[kListingDirectory]->Set(this, nPort, g_MemcardPath.c_str(), kListingEntryCount, 0);
    const char *pszFreqExt;
    g_pMemcardConfig->FindSymbol("freq_ext", &pszFreqExt, true);
    mGetDirs[kListingFreqs]->Set(this,
                                 nPort,
                                 FormatString("%s/*%s", g_MemcardPath.c_str(), pszFreqExt),
                                 kListingEntryCount,
                                 0);
}

void MCInitialCheckTask::OnDirListed() {
    if (mListing == kListingDirectory) {
        if (MemcardTask::sDirCount == 0) {
            mNeeded += g_nMemcardDirKilobytes;
        }
        ++mListing;
    } else if (mListing == kListingFreqs) {
        if (MemcardTask::sDirCount == 0) {
            ++mNeeded;
            int nFreqSize;
            g_pMemcardConfig->FindInt("max_freq_size", &nFreqSize, true);
            mNeeded += BytesToKilobytes(nFreqSize);
        }
        if (mGetInfo->mFree < mNeeded) {
            MemcardTask::sStatus = MemcardTask::kStatusFull;
        } else {
            MemcardTask::sStatus = MemcardTask::kStatusOk;
        }
    }
}

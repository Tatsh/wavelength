#include "memcard/mcloadfreqstask.h"

#include "memcard/memcardconfig.h"

namespace {

// The entries the listing requests.
constexpr int kListingEntryCount = 200;

} // namespace

MCLoadFreqsTask::MCLoadFreqsTask() {
    mGetInfo = new MCGetInfoTask;
    mGetDir = new MCGetDirTask;
    mLoadFiles = new MCLoadFreqFilesTask;
    Add(mGetInfo);
    Add(mGetDir);
    Add(mLoadFiles);
}

void MCLoadFreqsTask::Set(int nPort, std::vector<PlayerProfile> *pProfiles) {
    mGetInfo->Set(this, nPort, 0);
    const char *pszDirExt = nullptr;
    g_pMemcardConfig->FindSymbol("freq_dir_ext", &pszDirExt, true);
    mGetDir->Set(this,
                 nPort,
                 FormatString("%s%s/*%s", g_pszMemcardBaseDir, pszDirExt, g_pszFreqExt),
                 kListingEntryCount,
                 0);
    mLoadFiles->Set(nPort, pProfiles);
}

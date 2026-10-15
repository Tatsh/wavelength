#include "memcard/mccheckdirexiststask.h"

#ifdef VIDEO_STANDARD_PAL

#include <vector>

#include <libmc.h>

#include "memcard/memcardconfig.h"
#include "os/memcardsync.h"

namespace {

// The entries the listing requests.
constexpr int kListingEntryCount = 200;

} // namespace

void MCCheckDirExistsTask::Set(int nPort, const char *pszDir, MemcardSerialTask *pUser) {
    SetPort(nPort);
    mDir = pszDir != nullptr ? String(pszDir) : String(g_MemcardPath);
    mUser = pUser;
    mGetDir.Set(nullptr, nPort, FormatString("%s/*", mDir.c_str()), kListingEntryCount, 0);
}

void MCCheckDirExistsTask::OnStart() {
    mGetDir.Start();
}

void MCCheckDirExistsTask::CheckFiles(int *pbDamaged, int *pbWhole) {
    if (sDirCount == 0) {
        *pbDamaged = 0;
        *pbWhole = 0;
        return;
    }
    std::vector<String> names;
    std::vector<int> sizes;
    const char *pszIconFile;
    g_pMemcardConfig->FindSymbol("icon_file", &pszIconFile, true);
    names.push_back(String(pszIconFile));
    sizes.push_back(g_nMemcardIconSize);
    names.push_back(String("icon.sys"));
    sizes.push_back(sizeof(sceMcIconSys));
    names.push_back(mDir);
    sizes.push_back(kMemcardDirFileSize);
    unsigned int i = 0;
    for (; i < names.size(); ++i) {
        const MemcardDirEntry *pEntry = sDirEntries;
        const MemcardDirEntry *pEnd = sDirEntries + sDirCount;
        for (; pEntry < pEnd; ++pEntry) {
            if (names[i] == pEntry->mName && sizes[i] == pEntry->mSize) {
                break;
            }
        }
        if (pEntry == pEnd) {
            break;
        }
    }
    *pbDamaged = i != names.size();
    *pbWhole = !*pbDamaged;
}

void MCCheckDirExistsTask::OnPoll() {
    MemcardTask::OnPoll();
    const int nState = mGetDir.Poll();
    if (nState != kStateDone && nState != kStateFailed) {
        return;
    }
    int bDamaged;
    int bWhole;
    CheckFiles(&bDamaged, &bWhole);
    if (bDamaged) {
        MemcardDeleteDirAndWait(mPort, mDir.c_str());
        g_bMemcardDirExists = 0;
    }
    Finish(true);
    if (mUser != nullptr) {
        mUser->OnDirChecked(bWhole);
    }
}

#endif

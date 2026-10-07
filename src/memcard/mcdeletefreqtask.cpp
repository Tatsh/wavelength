#include "memcard/mcdeletefreqtask.h"

#include <ctype.h>

#include "memcard/memcardconfig.h"

namespace {

// Bytes of the lowered name, the terminator included.
constexpr int kNameBufferSize = 32;

} // namespace

MCDeleteFreqTask::MCDeleteFreqTask() {
    mGetInfo = new MCGetInfoTask;
    mDelete = new MCDeleteTask;
    Add(mGetInfo);
    Add(mDelete);
}

void MCDeleteFreqTask::Set(int nPort, const char *pszName) {
    mGetInfo->Set(nullptr, nPort, 0);
    const char *pszDirExt;
    g_pMemcardConfig->FindSymbol("freq_dir_ext", &pszDirExt, true);
    char szLowered[kNameBufferSize];
    char *pDest = szLowered;
    for (const char *pSource = pszName; *pSource != '\0'; ++pSource) {
        *pDest++ = tolower(*pSource);
    }
    *pDest = '\0';
    mDelete->Set(
        nPort, FormatString("%s%s/%s%s", g_pszMemcardBaseDir, pszDirExt, szLowered, g_pszFreqExt));
}

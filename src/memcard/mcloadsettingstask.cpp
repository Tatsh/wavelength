#include "memcard/mcloadsettingstask.h"

#include "memcard/memcardconfig.h"
#include "os/bufstream.h"

MCLoadSettingsTask::MCLoadSettingsTask() {
    mGetInfo = new MCGetInfoTask;
    mLoad = new MCLoadFileTask;
    Add(mGetInfo);
    Add(mLoad);
}

void MCLoadSettingsTask::Set(int nPort, GlobalSettings *pSettings) {
    mSettings = pSettings;
    mGetInfo->Set(nullptr, nPort, 0);
    const char *pszExt;
    g_pMemcardConfig->FindSymbol("settings_ext", &pszExt, true);
    mLoad->Set(this,
               nPort,
               FormatString("%s/settings%s", g_MemcardPath.c_str(), pszExt),
               g_pMemcardBuffer,
               g_nMemcardBufferSize);
}

void MCLoadSettingsTask::OnFileLoaded() {
    BufStream stream(g_pMemcardBuffer, g_nMemcardBufferSize, true);
    stream >> *mSettings;
}

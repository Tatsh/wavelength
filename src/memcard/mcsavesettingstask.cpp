#include "memcard/mcsavesettingstask.h"

#include "memcard/memcardconfig.h"
#include "memcard/memcardtask.h"
#include "os/bufstream.h"

namespace {

// The settings file is the only file of its extension the directory may include.
constexpr int kMaxSettingsFiles = 1;

} // namespace

MCSaveSettingsTask::MCSaveSettingsTask() : mName("settings") {
    mCheck = new MCCheckSaveSpaceNeededTask;
    mCreateDir = new MCCreateSaveDirTask;
    mSaveFile = new MCSaveFileTask;
    Add(mCheck);
    Add(mCreateDir);
    Add(mSaveFile);
}

void MCSaveSettingsTask::Set(int nPort, GlobalSettings *pSettings, int nOverwrite) {
    mPort = nPort;
    mNeeded = 0;
    BufStream stream(g_pMemcardBuffer, g_nMemcardBufferSize, true);
    stream << *pSettings;
    mSize = stream.Tell();
    const char *pszExt;
    g_pMemcardConfig->FindSymbol("settings_ext", &pszExt, true);
    mCheck->Set(this, nPort, mName.c_str(), pszExt, kMaxSettingsFiles, mSize, nOverwrite);
}

int MCSaveSettingsTask::GetNeeded() {
    return mNeeded;
}

void MCSaveSettingsTask::OnSaveSpace(int nNeeded) {
    if (MemcardTask::sStatus == MemcardTask::kStatusOk) {
        const char *pszLabel;
        g_pMemcardConfig->FindSymbol("freq_label", &pszLabel, true);
        mCreateDir->Set(mPort, pszLabel);
        const char *pszExt;
        g_pMemcardConfig->FindSymbol("settings_ext", &pszExt, true);
        mSaveFile->Set(mPort,
                       FormatString("%s/%s%s", g_MemcardPath.c_str(), mName.c_str(), pszExt),
                       g_pMemcardBuffer,
                       mSize);
    } else {
        mNeeded = nNeeded;
        mCheck->Finish(false);
    }
}

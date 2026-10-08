#include "memcard/mcmanager.h"

#include <string.h>

#include "memcard/memcard.h"
#include "memcard/memcardconfig.h"
#include "os/debug.h"
#include "os/locale.h"
#include "os/task.h"

// The unit's static initialiser at NTSC-U/C: 0x0015d0b8, PAL: 0x0015e8a8, and its global
// constructor at NTSC-U/C: 0x0015d218, PAL: 0x0015ea08, construct and destroy it.
MCManager TheMCManager;

void MCManager::Init() {
    LoadMemcardConfig();
    mInitialCheck = new MCInitialCheckTask;
    mSaveFreq = new MCSaveFreqTask;
    mLoadFreqs = new MCLoadFreqsTask;
    mDeleteFreq = new MCDeleteFreqTask;
    mSaveRemix = new MCSaveRemixTask;
    mLoadRemix = new MCLoadRemixTask;
    mListRemixes = new MCListRemixesTask;
    mDeleteRemix = new MCDeleteRemixTask;
    mSaveSettings = new MCSaveSettingsTask;
    mLoadSettings = new MCLoadSettingsTask;
    mSaveData = new MCSaveDataTask;
    mFormatCard = new MCFormatCardTask;
    mUnformatCard = new MCUnformatCardTask;
    mCardStatus = new MCCardStatusTask;
    mListNetConfigs = new MCListNetConfigsTask;
}

void MCManager::Terminate() {
    delete mListNetConfigs;
    delete mCardStatus;
    delete mUnformatCard;
    delete mFormatCard;
    delete mSaveData;
    delete mLoadSettings;
    delete mSaveSettings;
    delete mDeleteRemix;
    delete mListRemixes;
    delete mLoadRemix;
    delete mSaveRemix;
    delete mDeleteFreq;
    delete mLoadFreqs;
    delete mSaveFreq;
    delete mInitialCheck;
    FreeMemcardConfig();
}

void MCManager::InitialCheck(MemcardUser *pUser, int nSlot) {
    mInitialCheck->Set(nSlot);
    StartTask(mInitialCheck, pUser);
}

void MCManager::GetCardStatus(MemcardUser *pUser, int nSlot) {
    mCardStatus->Set(nSlot);
    StartTask(mCardStatus, pUser);
}

void MCManager::SaveFreq(
    MemcardUser *pUser, int nSlot, Campaign *pProfile, const char *pszOldName, int nOverwrite) {
    mSaveFreq->Set(nSlot, pProfile, pszOldName, nOverwrite);
    StartTask(mSaveFreq, pUser);
}

void MCManager::LoadFreqs(MemcardUser *pUser, int nSlot) {
    ClearResults();
    mLoadFreqs->Set(nSlot, &mProfiles);
    StartTask(mLoadFreqs, pUser);
}

void MCManager::DeleteFreq(MemcardUser *pUser, int nSlot, const char *pszName) {
    mDeleteFreq->Set(nSlot, pszName);
    StartTask(mDeleteFreq, pUser);
}

void MCManager::SaveRemix(
    MemcardUser *pUser, int nSlot, RemixInfo *pInfo, const void *pData, int nOverwrite) {
    mSaveRemix->Set(nSlot, pInfo->mName, pInfo, pData, pInfo->mDataSize, nOverwrite);
    StartTask(mSaveRemix, pUser);
}

void MCManager::LoadRemix(
    MemcardUser *pUser, int nSlot, const char *pszName, void *pBuffer, int nSize) {
    mLoadRemix->Set(nSlot, pszName, pBuffer, nSize);
    StartTask(mLoadRemix, pUser);
}

void MCManager::ListRemixes(MemcardUser *pUser, int nSlot) {
    mListRemixes->Set(nSlot, &mRemixInfos);
    StartTask(mListRemixes, pUser);
}

void MCManager::DeleteRemix(MemcardUser *pUser, int nSlot, const char *pszName) {
    mDeleteRemix->Set(nSlot, pszName);
    StartTask(mDeleteRemix, pUser);
}

void MCManager::SaveSettings(MemcardUser *pUser,
                             int nSlot,
                             GameOptions *pSettings,
                             int nOverwrite) {
    mSaveSettings->Set(nSlot, pSettings, nOverwrite);
    StartTask(mSaveSettings, pUser);
}

void MCManager::LoadSettings(MemcardUser *pUser, int nSlot) {
    mLoadSettings->Set(nSlot, &mSettings);
    StartTask(mLoadSettings, pUser);
}

void MCManager::ListNetConfigs(MemcardUser *pUser, int nSlot) {
    mListNetConfigs->Set(nSlot);
    StartTask(mListNetConfigs, pUser);
}

void MCManager::FormatCard(MemcardUser *pUser, int nSlot) {
    mFormatCard->Set(nSlot);
    StartTask(mFormatCard, pUser);
}

void MCManager::SaveFile(MemcardUser *pUser,
                         int nSlot,
                         const char *pszName,
                         const void *pData,
                         int nSize,
                         [[maybe_unused]] bool bReserved) {
    mSaveData->Set(nSlot, pszName, pData, nSize);
    StartTask(mSaveData, pUser);
}

void MCManager::StartTask(MemcardSerialTask *pTask, MemcardUser *pUser) {
    mUser = pUser;
    mCurrentTask = pTask;
    pTask->Start();
}

void MCManager::ClearTask() {
    mCurrentTask = nullptr;
    mUser = nullptr;
}

void MCManager::ClearResults() {
    mProfiles.clear();
    mRemixInfos.clear();
    mReserved80.clear();
    GameOptions settings; // Yes, the binary constructs this local and never reads it.
}

void MCManager::Poll() {
    if (mCurrentTask == nullptr) {
        return;
    }
    const int nState = mCurrentTask->Poll();
    if (nState != Task::kStateDone && nState != Task::kStateFailed) {
        return;
    }
    const int nStatus = mCurrentTask->GetResult();
    if (mCurrentTask == mSaveFreq) {
        mUser->OnFreqSaved(nStatus, mSaveFreq->GetNeeded());
    } else if (mCurrentTask == mLoadFreqs) {
        mUser->OnFreqsLoaded(nStatus, &mProfiles);
    } else if (mCurrentTask == mDeleteFreq) {
        mUser->OnFreqDeleted(nStatus);
    } else if (mCurrentTask == mSaveRemix) {
        mUser->OnRemixSaved(nStatus, mSaveRemix->GetNeeded());
    } else if (mCurrentTask == mLoadRemix) {
        mUser->OnRemixLoaded(nStatus);
    } else if (mCurrentTask == mListRemixes) {
        mUser->OnRemixesListed(nStatus, &mRemixInfos);
    } else if (mCurrentTask == mDeleteRemix) {
        mUser->OnRemixDeleted(nStatus);
    } else if (mCurrentTask == mSaveSettings) {
        mUser->OnSettingsSaved(nStatus, mSaveSettings->GetNeeded());
    } else if (mCurrentTask == mLoadSettings) {
        mUser->OnSettingsLoaded(nStatus, mSettings);
    } else if (mCurrentTask == mInitialCheck) {
        mUser->OnInitialCheck(nStatus,
                              mInitialCheck->mGetInfo->mFormat,
                              mInitialCheck->mGetInfo->mFree,
                              mInitialCheck->mNeeded);
    } else if (mCurrentTask == mCardStatus) {
        mUser->OnCardStatus(nStatus);
    } else if (mCurrentTask == mSaveData) {
        mUser->OnFileSaved(nStatus);
    } else if (mCurrentTask == mListNetConfigs) {
        mUser->OnNetConfigsListed(nStatus, mListNetConfigs->GetConfigs());
    } else if (mCurrentTask == mFormatCard) {
        mUser->OnCardFormatted(nStatus);
    } else if (mCurrentTask == mUnformatCard) {
        mUser->OnCardUnformatted(nStatus);
    } else {
        DebugWarn(" ERROR: bad task!!!\n");
    }
    mCurrentTask->Stop();
    ClearTask();
}

const char *MCManager::GetSlotName(int nSlot) {
    const char *pszToken = MemcardGetSlotName(nSlot);
    if (strcmp(pszToken, "") == 0) {
        return "";
    }
    return TheLocale.Localize(pszToken, true);
}

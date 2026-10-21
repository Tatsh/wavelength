#include "memcard/mcmanager.h"

#include <string.h>

#include "memcard/memcard.h"
#include "memcard/memcardconfig.h"
#include "os/debug.h"
#include "os/locale.h"
#include "os/task.h"

// The unit's static initialiser at NTSC-U/C: 0x0015d0b8, PAL: 0x0015e8a8, its global
// constructor at NTSC-U/C: 0x0015d218, PAL: 0x0015ea08, and its global destructor at
// NTSC-U/C: 0x0015d238, PAL: 0x0015ea28, construct and destroy it.
MCManager TheMCManager;

void MCManager::Init() {
    LoadMemcardConfig();
    mInitialCheck = std::make_unique<MCInitialCheckTask>();
    mSaveFreq = std::make_unique<MCSaveFreqTask>();
    mLoadFreqs = std::make_unique<MCLoadFreqsTask>();
    mDeleteFreq = std::make_unique<MCDeleteFreqTask>();
    mSaveRemix = std::make_unique<MCSaveRemixTask>();
    mLoadRemix = std::make_unique<MCLoadRemixTask>();
    mListRemixes = std::make_unique<MCListRemixesTask>();
    mDeleteRemix = std::make_unique<MCDeleteRemixTask>();
    mSaveSettings = std::make_unique<MCSaveSettingsTask>();
    mLoadSettings = std::make_unique<MCLoadSettingsTask>();
    mSaveData = std::make_unique<MCSaveDataTask>();
    mFormatCard = std::make_unique<MCFormatCardTask>();
    mUnformatCard = std::make_unique<MCUnformatCardTask>();
    mCardStatus = std::make_unique<MCCardStatusTask>();
    mListNetConfigs = std::make_unique<MCListNetConfigsTask>();
}

void MCManager::Terminate() {
    mListNetConfigs.reset();
    mCardStatus.reset();
    mUnformatCard.reset();
    mFormatCard.reset();
    mSaveData.reset();
    mLoadSettings.reset();
    mSaveSettings.reset();
    mDeleteRemix.reset();
    mListRemixes.reset();
    mLoadRemix.reset();
    mSaveRemix.reset();
    mDeleteFreq.reset();
    mLoadFreqs.reset();
    mSaveFreq.reset();
    mInitialCheck.reset();
    FreeMemcardConfig();
}

void MCManager::InitialCheck(MemcardUser *pUser, int nSlot) {
    mInitialCheck->Set(nSlot);
    StartTask(mInitialCheck.get(), pUser);
}

void MCManager::GetCardStatus(MemcardUser *pUser, int nSlot) {
    mCardStatus->Set(nSlot);
    StartTask(mCardStatus.get(), pUser);
}

void MCManager::SaveFreq(
    MemcardUser *pUser, int nSlot, Campaign *pProfile, const char *pszOldName, int nOverwrite) {
    mSaveFreq->Set(nSlot, pProfile, pszOldName, nOverwrite);
    StartTask(mSaveFreq.get(), pUser);
}

void MCManager::LoadFreqs(MemcardUser *pUser, int nSlot) {
    ClearResults();
    mLoadFreqs->Set(nSlot, &mProfiles);
    StartTask(mLoadFreqs.get(), pUser);
}

void MCManager::DeleteFreq(MemcardUser *pUser, int nSlot, const char *pszName) {
    mDeleteFreq->Set(nSlot, pszName);
    StartTask(mDeleteFreq.get(), pUser);
}

void MCManager::SaveRemix(
    MemcardUser *pUser, int nSlot, RemixInfo *pInfo, const void *pData, int nOverwrite) {
    mSaveRemix->Set(nSlot, pInfo->mName, pInfo, pData, pInfo->mDataSize, nOverwrite);
    StartTask(mSaveRemix.get(), pUser);
}

void MCManager::LoadRemix(
    MemcardUser *pUser, int nSlot, const char *pszName, void *pBuffer, int nSize) {
    mLoadRemix->Set(nSlot, pszName, pBuffer, nSize);
    StartTask(mLoadRemix.get(), pUser);
}

void MCManager::ListRemixes(MemcardUser *pUser, int nSlot) {
    mListRemixes->Set(nSlot, &mRemixInfos);
    StartTask(mListRemixes.get(), pUser);
}

void MCManager::DeleteRemix(MemcardUser *pUser, int nSlot, const char *pszName) {
    mDeleteRemix->Set(nSlot, pszName);
    StartTask(mDeleteRemix.get(), pUser);
}

void MCManager::SaveSettings(MemcardUser *pUser,
                             int nSlot,
                             GameOptions *pSettings,
                             int nOverwrite) {
    mSaveSettings->Set(nSlot, pSettings, nOverwrite);
    StartTask(mSaveSettings.get(), pUser);
}

void MCManager::LoadSettings(MemcardUser *pUser, int nSlot) {
    mLoadSettings->Set(nSlot, &mSettings);
    StartTask(mLoadSettings.get(), pUser);
}

void MCManager::ListNetConfigs(MemcardUser *pUser, int nSlot) {
    mListNetConfigs->Set(nSlot);
    StartTask(mListNetConfigs.get(), pUser);
}

void MCManager::FormatCard(MemcardUser *pUser, int nSlot) {
    mFormatCard->Set(nSlot);
    StartTask(mFormatCard.get(), pUser);
}

void MCManager::SaveFile(MemcardUser *pUser,
                         int nSlot,
                         const char *pszName,
                         const void *pData,
                         int nSize,
                         [[maybe_unused]] bool bReserved) {
    mSaveData->Set(nSlot, pszName, pData, nSize);
    StartTask(mSaveData.get(), pUser);
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
    if (mCurrentTask == mSaveFreq.get()) {
        mUser->OnFreqSaved(nStatus, mSaveFreq->GetNeeded());
    } else if (mCurrentTask == mLoadFreqs.get()) {
        mUser->OnFreqsLoaded(nStatus, &mProfiles);
    } else if (mCurrentTask == mDeleteFreq.get()) {
        mUser->OnFreqDeleted(nStatus);
    } else if (mCurrentTask == mSaveRemix.get()) {
        mUser->OnRemixSaved(nStatus, mSaveRemix->GetNeeded());
    } else if (mCurrentTask == mLoadRemix.get()) {
        mUser->OnRemixLoaded(nStatus);
    } else if (mCurrentTask == mListRemixes.get()) {
        mUser->OnRemixesListed(nStatus, &mRemixInfos);
    } else if (mCurrentTask == mDeleteRemix.get()) {
        mUser->OnRemixDeleted(nStatus);
    } else if (mCurrentTask == mSaveSettings.get()) {
        mUser->OnSettingsSaved(nStatus, mSaveSettings->GetNeeded());
    } else if (mCurrentTask == mLoadSettings.get()) {
        mUser->OnSettingsLoaded(nStatus, mSettings);
    } else if (mCurrentTask == mInitialCheck.get()) {
        mUser->OnInitialCheck(nStatus,
                              mInitialCheck->GetGetInfo()->GetFormat(),
                              mInitialCheck->GetGetInfo()->GetFree(),
                              mInitialCheck->GetNeeded());
    } else if (mCurrentTask == mCardStatus.get()) {
        mUser->OnCardStatus(nStatus);
    } else if (mCurrentTask == mSaveData.get()) {
        mUser->OnFileSaved(nStatus);
    } else if (mCurrentTask == mListNetConfigs.get()) {
        mUser->OnNetConfigsListed(nStatus, mListNetConfigs->GetConfigs());
    } else if (mCurrentTask == mFormatCard.get()) {
        mUser->OnCardFormatted(nStatus);
    } else if (mCurrentTask == mUnformatCard.get()) {
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

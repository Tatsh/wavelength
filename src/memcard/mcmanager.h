#pragma once

#include <vector>

#include "game/campaign.h"
#include "game/globalsettings.h"
#include "game/remixinfo.h"
#include "memcard/mccardstatustask.h"
#include "memcard/mcdeletefreqtask.h"
#include "memcard/mcdeleteremixtask.h"
#include "memcard/mcformatcardtask.h"
#include "memcard/mcinitialchecktask.h"
#include "memcard/mclistnetconfigstask.h"
#include "memcard/mclistremixestask.h"
#include "memcard/mcloadfreqstask.h"
#include "memcard/mcloadremixtask.h"
#include "memcard/mcloadsettingstask.h"
#include "memcard/mcsavedatatask.h"
#include "memcard/mcsavefreqtask.h"
#include "memcard/mcsaveremixtask.h"
#include "memcard/mcsavesettingstask.h"
#include "memcard/mcunformatcardtask.h"
#include "memcard/memcardserialtask.h"
#include "memcard/memcarduser.h"

/**
 * Front end of the memory card work. It runs one piece of work at a time on behalf of a
 * MemcardUser.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the tasks it runs. The
 * object is 0x90 bytes. Each starting routine prepares one task and makes it the running task.
 * Poll() reports the end of the running task to its user through the MemcardUser method for the
 * task, together with the results the manager retains.
 */
class MCManager {
public:
    /** Construct a manager with no task running. The task objects are built by Init(). */
    MCManager() : mUser(nullptr) {
    }

    /**
     * Read the memory card configuration and build the tasks.
     *
     * Metagame::Init() calls it.
     *
     * @ghidraAddress NTSC-U/C: 0x0015c400
     * @ghidraAddress PAL: 0x0015dbf0
     */
    void Init();

    /**
     * Destroy the tasks Init() built, and release the configuration buffers.
     *
     * @ghidraAddress NTSC-U/C: 0x0015c550
     * @ghidraAddress PAL: 0x0015dd40
     */
    void Terminate();

    /**
     * Start checking a card at startup. MemcardUser::OnInitialCheck() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015c758
     * @ghidraAddress PAL: 0x0015df48
     */
    void InitialCheck(MemcardUser *pUser, int nSlot);

    /**
     * Start learning whether a usable card is in a slot. MemcardUser::OnCardStatus() learns the
     * outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015c7a0
     * @ghidraAddress PAL: 0x0015df90
     */
    void GetCardStatus(MemcardUser *pUser, int nSlot);

    /**
     * Start saving the Freq of a profile. MemcardUser::OnFreqSaved() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @param pProfile The profile, saved under its name.
     * @param pszOldName The name the Freq was saved under before. A different name has its file
     *                   deleted.
     * @param nOverwrite Non-zero when a saved Freq of the name may be replaced.
     * @ghidraAddress NTSC-U/C: 0x0015c7e8
     * @ghidraAddress PAL: 0x0015dfd8
     */
    void SaveFreq(
        MemcardUser *pUser, int nSlot, Campaign *pProfile, const char *pszOldName, int nOverwrite);

    /**
     * Start loading every saved Freq into mProfiles. MemcardUser::OnFreqsLoaded() learns the
     * outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015c840
     * @ghidraAddress PAL: 0x0015e030
     */
    void LoadFreqs(MemcardUser *pUser, int nSlot);

    /**
     * Start deleting a saved Freq. MemcardUser::OnFreqDeleted() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @param pszName The Freq's name.
     * @ghidraAddress NTSC-U/C: 0x0015c8a0
     * @ghidraAddress PAL: 0x0015e090
     */
    void DeleteFreq(MemcardUser *pUser, int nSlot, const char *pszName);

    /**
     * Start saving a remix. MemcardUser::OnRemixSaved() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @param pInfo The description of the remix. Its name and size are those of the file and of
     *              the remix.
     * @param pData The remix.
     * @param nOverwrite Non-zero when a saved remix of the name may be replaced.
     * @ghidraAddress NTSC-U/C: 0x0015c8f0
     * @ghidraAddress PAL: 0x0015e0e0
     */
    void
    SaveRemix(MemcardUser *pUser, int nSlot, RemixInfo *pInfo, const void *pData, int nOverwrite);

    /**
     * Start loading a saved remix. MemcardUser::OnRemixLoaded() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @param pszName The name of the remix.
     * @param pBuffer Receives the remix. The caller retains it.
     * @param nSize The number of bytes to read.
     * @ghidraAddress NTSC-U/C: 0x0015c948
     * @ghidraAddress PAL: 0x0015e138
     */
    void LoadRemix(MemcardUser *pUser, int nSlot, const char *pszName, void *pBuffer, int nSize);

    /**
     * Start reading the descriptions of the saved remixes into mRemixInfos.
     * MemcardUser::OnRemixesListed() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015c9a0
     * @ghidraAddress PAL: 0x0015e190
     */
    void ListRemixes(MemcardUser *pUser, int nSlot);

    /**
     * Start deleting a saved remix. MemcardUser::OnRemixDeleted() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @param pszName The name of the remix.
     * @ghidraAddress NTSC-U/C: 0x0015c9f0
     * @ghidraAddress PAL: 0x0015e1e0
     */
    void DeleteRemix(MemcardUser *pUser, int nSlot, const char *pszName);

    /**
     * Start saving the settings. MemcardUser::OnSettingsSaved() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @param pSettings The settings.
     * @param nOverwrite Non-zero when saved settings may be replaced.
     * @ghidraAddress NTSC-U/C: 0x0015ca40
     * @ghidraAddress PAL: 0x0015e230
     */
    void SaveSettings(MemcardUser *pUser, int nSlot, GlobalSettings *pSettings, int nOverwrite);

    /**
     * Start loading the settings into mSettings. MemcardUser::OnSettingsLoaded() learns the
     * outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015ca90
     * @ghidraAddress PAL: 0x0015e280
     */
    void LoadSettings(MemcardUser *pUser, int nSlot);

    /**
     * Start listing the network configurations. MemcardUser::OnNetConfigsListed() learns the
     * outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015cae0
     * @ghidraAddress PAL: 0x0015e2d0
     */
    void ListNetConfigs(MemcardUser *pUser, int nSlot);

    /**
     * Start formatting an unformatted card. MemcardUser::OnCardFormatted() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015cb28
     * @ghidraAddress PAL: 0x0015e318
     */
    void FormatCard(MemcardUser *pUser, int nSlot);

    /**
     * Start saving a block of memory to a file. MemcardUser::OnFileSaved() learns the outcome.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @param pszName The name of the file.
     * @param pData The data.
     * @param nSize The number of bytes.
     * @param bReserved Passed by every caller. The body does not read it.
     * @ghidraAddress NTSC-U/C: 0x0015cb70
     * @ghidraAddress PAL: 0x0015e360
     */
    void SaveFile(MemcardUser *pUser,
                  int nSlot,
                  const char *pszName,
                  const void *pData,
                  int nSize,
                  bool bReserved);

    /**
     * Report the end of the running task to its user, and stop the task.
     *
     * Metagame::Update() calls it every frame. A running task that none of the tasks matches is
     * reported as `ERROR: bad task!!!`.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ccd8
     * @ghidraAddress PAL: 0x0015e4c8
     */
    void Poll();

    /**
     * Report the localised name of a memory card slot.
     *
     * @param nSlot The memory card slot.
     * @return The name, or an empty string for a slot without a name.
     * @ghidraAddress NTSC-U/C: 0x0015d050
     * @ghidraAddress PAL: 0x0015e840
     */
    const char *GetSlotName(int nSlot);

    MemcardUser *mUser;                    /*!< The user of the running task. */
    MCInitialCheckTask *mInitialCheck;     /*!< The work of InitialCheck(). */
    MCCardStatusTask *mCardStatus;         /*!< The work of GetCardStatus(). */
    MCSaveFreqTask *mSaveFreq;             /*!< The work of SaveFreq(). */
    MCLoadFreqsTask *mLoadFreqs;           /*!< The work of LoadFreqs(). */
    MCDeleteFreqTask *mDeleteFreq;         /*!< The work of DeleteFreq(). */
    MCSaveRemixTask *mSaveRemix;           /*!< The work of SaveRemix(). */
    MCLoadRemixTask *mLoadRemix;           /*!< The work of LoadRemix(). */
    MCListRemixesTask *mListRemixes;       /*!< The work of ListRemixes(). */
    MCDeleteRemixTask *mDeleteRemix;       /*!< The work of DeleteRemix(). */
    MCSaveSettingsTask *mSaveSettings;     /*!< The work of SaveSettings(). */
    MCLoadSettingsTask *mLoadSettings;     /*!< The work of LoadSettings(). */
    MCSaveDataTask *mSaveData;             /*!< The work of SaveFile(). */
    MCFormatCardTask *mFormatCard;         /*!< The work of FormatCard(). */
    MCUnformatCardTask *mUnformatCard;     /*!< Unformat work. No routine starts it. */
    MCListNetConfigsTask *mListNetConfigs; /*!< The work of ListNetConfigs(). */
    MemcardSerialTask *mCurrentTask;       /*!< The running task, or null. */
    std::vector<Campaign> mProfiles;       /*!< The profiles LoadFreqs() loads. */
    std::vector<RemixInfo> mRemixInfos;    /*!< The descriptions ListRemixes() reads. */
    GlobalSettings mSettings;              /*!< The settings LoadSettings() loads. */
    std::vector<int> mReserved80;          // +0x80, emptied by ClearResults() and never read.

private:
    /**
     * Make a task the running task and start it.
     *
     * @param pTask The task.
     * @param pUser The user to report to.
     * @ghidraAddress NTSC-U/C: 0x0015cbc8
     * @ghidraAddress PAL: 0x0015e3b8
     */
    void StartTask(MemcardSerialTask *pTask, MemcardUser *pUser);

    /**
     * Forget the running task and its user.
     *
     * @ghidraAddress NTSC-U/C: 0x0015cbf0
     * @ghidraAddress PAL: 0x0015e3e0
     */
    void ClearTask();

    /**
     * Empty the results the manager retains.
     *
     * @ghidraAddress NTSC-U/C: 0x0015cc00
     * @ghidraAddress PAL: 0x0015e3f0
     */
    void ClearResults();
};

/**
 * The memory card front end.
 *
 * @ghidraAddress NTSC-U/C: 0x004362b0
 */
extern MCManager TheMCManager;

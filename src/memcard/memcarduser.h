#pragma once

#include <list>
#include <vector>

#include "game/globalsettings.h"
#include "game/playerprofile.h"
#include "game/remixinfo.h"
#include "netflow/inetconfig.h"

/**
 * Receiver of the outcome of the memory card work MCManager runs.
 *
 * The RTTI includes the class name and records no base. The vptr is the only member, and the
 * class has no virtual destructor. MCManager::Poll() reports the end of each piece of work through
 * the one method for it. Every method does nothing here. Each status is one of
 * MemcardTask::Status.
 */
class MemcardUser {
public:
    /**
     * Learn the outcome of MCManager::InitialCheck().
     *
     * @param nStatus The outcome.
     * @param nFormat Non-zero for a formatted card.
     * @param nFree The free space of the card in kilobytes.
     * @param nNeeded The kilobytes one more Freq needs.
     * @ghidraAddress NTSC-U/C: 0x00341db0
     * @ghidraAddress PAL: 0x003af2e8
     */
    virtual void OnInitialCheck([[maybe_unused]] int nStatus,
                                [[maybe_unused]] int nFormat,
                                [[maybe_unused]] int nFree,
                                [[maybe_unused]] int nNeeded) {
    }

    /**
     * Learn the outcome of MCManager::GetCardStatus().
     *
     * @param nStatus The outcome.
     * @ghidraAddress NTSC-U/C: 0x00341db8
     * @ghidraAddress PAL: 0x003af2f0
     */
    virtual void OnCardStatus([[maybe_unused]] int nStatus) {
    }

    /**
     * Learn the outcome of MCManager::SaveFreq().
     *
     * @param nStatus The outcome.
     * @param nNeeded The kilobytes a save that did not fit needed, or zero.
     * @ghidraAddress NTSC-U/C: 0x00341dc0
     * @ghidraAddress PAL: 0x003af2f8
     */
    virtual void OnFreqSaved([[maybe_unused]] int nStatus, [[maybe_unused]] int nNeeded) {
    }

    /**
     * Learn the outcome of MCManager::LoadFreqs().
     *
     * @param nStatus The outcome.
     * @param pProfiles The profiles loaded, newest first. MCManager retains them.
     * @ghidraAddress NTSC-U/C: 0x00341dc8
     * @ghidraAddress PAL: 0x003af300
     */
    virtual void OnFreqsLoaded([[maybe_unused]] int nStatus,
                               [[maybe_unused]] std::vector<PlayerProfile> *pProfiles) {
    }

    /**
     * Learn the outcome of MCManager::DeleteFreq().
     *
     * @param nStatus The outcome.
     * @ghidraAddress NTSC-U/C: 0x00341dd0
     * @ghidraAddress PAL: 0x003af308
     */
    virtual void OnFreqDeleted([[maybe_unused]] int nStatus) {
    }

    /**
     * Do nothing.
     *
     * No work reports through this vtable slot and no receiver overrides it. The arguments cannot
     * be established and are not declared.
     *
     * @ghidraAddress NTSC-U/C: 0x00341dd8
     * @ghidraAddress PAL: 0x003af310
     */
    virtual void UnusedReport() {
    }

    /**
     * Learn the outcome of MCManager::SaveRemix().
     *
     * @param nStatus The outcome.
     * @param nNeeded The kilobytes a save that did not fit needed, or zero.
     * @ghidraAddress NTSC-U/C: 0x00341de0
     * @ghidraAddress PAL: 0x003af318
     */
    virtual void OnRemixSaved([[maybe_unused]] int nStatus, [[maybe_unused]] int nNeeded) {
    }

    /**
     * Learn the outcome of MCManager::LoadRemix().
     *
     * @param nStatus The outcome.
     * @ghidraAddress NTSC-U/C: 0x00341de8
     * @ghidraAddress PAL: 0x003af320
     */
    virtual void OnRemixLoaded([[maybe_unused]] int nStatus) {
    }

    /**
     * Learn the outcome of MCManager::ListRemixes().
     *
     * @param nStatus The outcome.
     * @param pInfos The descriptions of the remixes. MCManager retains them.
     * @ghidraAddress NTSC-U/C: 0x00341df0
     * @ghidraAddress PAL: 0x003af328
     */
    virtual void OnRemixesListed([[maybe_unused]] int nStatus,
                                 [[maybe_unused]] std::vector<RemixInfo> *pInfos) {
    }

    /**
     * Learn the outcome of MCManager::DeleteRemix().
     *
     * @param nStatus The outcome.
     * @ghidraAddress NTSC-U/C: 0x00341df8
     * @ghidraAddress PAL: 0x003af330
     */
    virtual void OnRemixDeleted([[maybe_unused]] int nStatus) {
    }

    /**
     * Learn the outcome of MCManager::SaveSettings().
     *
     * @param nStatus The outcome.
     * @param nNeeded The kilobytes a save that did not fit needed, or zero.
     * @ghidraAddress NTSC-U/C: 0x00341e00
     * @ghidraAddress PAL: 0x003af338
     */
    virtual void OnSettingsSaved([[maybe_unused]] int nStatus, [[maybe_unused]] int nNeeded) {
    }

    /**
     * Learn the outcome of MCManager::LoadSettings().
     *
     * @param nStatus The outcome.
     * @param settings A copy of the settings loaded.
     * @ghidraAddress NTSC-U/C: 0x00341e08
     * @ghidraAddress PAL: 0x003af340
     */
    virtual void OnSettingsLoaded([[maybe_unused]] int nStatus,
                                  [[maybe_unused]] GlobalSettings settings) {
    }

    /**
     * Learn the outcome of MCManager::ListNetConfigs().
     *
     * @param nStatus The outcome.
     * @param pConfigs The network configurations. The work retains them.
     * @ghidraAddress NTSC-U/C: 0x00341e50
     * @ghidraAddress PAL: 0x003af388
     */
    virtual void OnNetConfigsListed([[maybe_unused]] int nStatus,
                                    [[maybe_unused]] std::list<InetConfig> *pConfigs) {
    }

    /**
     * Learn the outcome of MCManager::FormatCard().
     *
     * @param nStatus The outcome.
     * @ghidraAddress NTSC-U/C: 0x00341e58
     * @ghidraAddress PAL: 0x003af390
     */
    virtual void OnCardFormatted([[maybe_unused]] int nStatus) {
    }

    /**
     * Learn the outcome of the unformat work. The game never starts the work.
     *
     * @param nStatus The outcome.
     * @ghidraAddress NTSC-U/C: 0x00341e60
     * @ghidraAddress PAL: 0x003af398
     */
    virtual void OnCardUnformatted([[maybe_unused]] int nStatus) {
    }

    /**
     * Learn the outcome of MCManager::SaveFile().
     *
     * @param nStatus The outcome.
     * @ghidraAddress NTSC-U/C: 0x00358ab8
     */
    virtual void OnFileSaved([[maybe_unused]] int nStatus) {
    }
};

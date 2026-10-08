#pragma once

#include <vector>

#include "game/campaign.h"
#include "memcard/mcloadfiletask.h"
#include "memcard/memcardtask.h"

/**
 * Task that loads every Freq file the last directory listing found into a list of profiles.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x4c bytes. The profiles
 * are in order of their files' last change, newest first, and the files load in that order.
 */
class MCLoadFreqFilesTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @param pProfiles Receives one profile for each file.
     * @ghidraAddress NTSC-U/C: 0x00160500
     * @ghidraAddress PAL: 0x00162d60
     */
    void Set(int nPort, std::vector<Campaign> *pProfiles);

    int mCardPort; /*!< The memory card slot the files load from. */
    int mIndex;    /*!< The index into MemcardTask::sDirEntries of the file loading now. */
    std::vector<Campaign> *mProfiles; /*!< Receives the profiles. */
    MCLoadFileTask mLoad;             /*!< The step that loads one file. */

protected:
    /**
     * Size the profiles to the files, sort the files, and start loading the newest.
     *
     * @ghidraAddress NTSC-U/C: 0x00160510
     */
    void OnStart() override;

    /**
     * Decode a loaded file into its profile and start loading the next.
     *
     * @ghidraAddress NTSC-U/C: 0x001606f0
     */
    void OnPoll() override;

private:
    /**
     * Start loading the file at mIndex, or finish once every file is loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00160630
     * @ghidraAddress PAL: 0x00162e28
     */
    void LoadNext();
};

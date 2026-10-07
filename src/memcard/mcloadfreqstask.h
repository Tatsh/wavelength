#pragma once

#include <vector>

#include "game/playerprofile.h"
#include "memcard/mcgetdirtask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/mcloadfreqfilestask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that loads every saved Freq: read the card information, list the Freq files, and load them.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x2c bytes.
 */
class MCLoadFreqsTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00160850
     * @ghidraAddress PAL: 0x001631e0
     */
    MCLoadFreqsTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pProfiles Receives one profile for each Freq.
     * @ghidraAddress NTSC-U/C: 0x00160980
     * @ghidraAddress PAL: 0x00163310
     */
    void Set(int nPort, std::vector<PlayerProfile> *pProfiles);

    MCGetInfoTask *mGetInfo;         /*!< The step that reads the card information. */
    MCGetDirTask *mGetDir;           /*!< The step that lists the Freq files. */
    MCLoadFreqFilesTask *mLoadFiles; /*!< The step that loads the Freq files. */
};

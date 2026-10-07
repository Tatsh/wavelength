#pragma once

#include <vector>

#include "game/remixinfo.h"
#include "memcard/mcgetallremixinfostask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/mcgetremixnamestask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that lists the saved remixes: read the card information, list the remix files, and read
 * their descriptions.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x2c bytes.
 */
class MCListRemixesTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00161c70
     * @ghidraAddress PAL: 0x00164980
     */
    MCListRemixesTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pInfos Receives one description for each remix.
     * @ghidraAddress NTSC-U/C: 0x00161de0
     * @ghidraAddress PAL: 0x00164af0
     */
    void Set(int nPort, std::vector<RemixInfo> *pInfos);

    MCGetInfoTask *mGetInfo;        /*!< The step that reads the card information. */
    MCGetRemixNamesTask *mNames;    /*!< The step that lists the remix files. */
    MCGetAllRemixInfosTask *mInfos; /*!< The step that reads the descriptions. */
};

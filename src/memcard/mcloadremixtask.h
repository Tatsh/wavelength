#pragma once

#include "memcard/mcfindremixtask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/mcgetremixdatatask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that loads a saved remix: read the card information, find the remix file, and read the
 * remix.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x2c bytes.
 */
class MCLoadRemixTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00161ae8
     * @ghidraAddress PAL: 0x001647f8
     */
    MCLoadRemixTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pszName The name of the remix file, without its extension.
     * @param pBuffer Receives the remix. The caller retains it.
     * @param nSize The number of bytes to read.
     * @ghidraAddress NTSC-U/C: 0x00161ba0
     * @ghidraAddress PAL: 0x001648b0
     */
    void Set(int nPort, const char *pszName, void *pBuffer, int nSize);

    MCGetInfoTask *mGetInfo;   /*!< The step that reads the card information. */
    MCFindRemixTask *mFind;    /*!< The step that finds the remix file. */
    MCGetRemixDataTask *mData; /*!< The work that reads the remix. */
};

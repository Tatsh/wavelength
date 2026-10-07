#pragma once

#include "memcard/mcdeletesavedremixtask.h"
#include "memcard/mcfindremixtask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that deletes a saved remix: read the card information, find the remix file, and delete it.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x30 bytes.
 */
class MCDeleteRemixTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00161e48
     * @ghidraAddress PAL: 0x00164b58
     */
    MCDeleteRemixTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pszName The name of the remix file, without its extension.
     * @ghidraAddress NTSC-U/C: 0x00161f80
     * @ghidraAddress PAL: 0x00164c90
     */
    void Set(int nPort, const char *pszName);

    MCGetInfoTask *mGetInfo;         /*!< The step that reads the card information. */
    int mReserved24;                 // +0x24, never written or read.
    MCFindRemixTask *mFind;          /*!< The step that finds the remix file. */
    MCDeleteSavedRemixTask *mDelete; /*!< The step that deletes the file. */
};

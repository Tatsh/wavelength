#pragma once

#include "memcard/mcgetinfotask.h"
#include "memcard/mcsavefiletask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that saves a block of memory to a file: read the card information, then save the file.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x28 bytes.
 * Another card than before counts as success.
 */
class MCSaveDataTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ffa0
     * @ghidraAddress PAL: 0x00162800
     */
    MCSaveDataTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pszName The file.
     * @param pData The bytes. The caller retains them.
     * @param nSize The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x00160038
     * @ghidraAddress PAL: 0x00162898
     */
    void Set(int nPort, const char *pszName, const void *pData, int nSize);

    MCGetInfoTask *mGetInfo;   /*!< The step that reads the card information. */
    MCSaveFileTask *mSaveFile; /*!< The work that saves the file. */
};

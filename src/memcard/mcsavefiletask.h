#pragma once

#include "memcard/mcclosetask.h"
#include "memcard/mcopentask.h"
#include "memcard/mcwritetask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that creates or replaces a file with a block of memory: open, write, and close.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x2c bytes.
 */
class MCSaveFileTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e330
     * @ghidraAddress PAL: 0x0015fb70
     */
    MCSaveFileTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pszName The file.
     * @param pData The bytes. The caller retains them.
     * @param nSize The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x0015e440
     * @ghidraAddress PAL: 0x0015fc80
     */
    void Set(int nPort, const char *pszName, const void *pData, int nSize);

    MCOpenTask *mOpen;   /*!< The step that opens the file. */
    MCWriteTask *mWrite; /*!< The step that writes the file. */
    MCCloseTask *mClose; /*!< The step that closes the file. */
};

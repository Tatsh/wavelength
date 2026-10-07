#pragma once

#include "memcard/mcclosetask.h"
#include "memcard/mcopentask.h"
#include "memcard/mcreadtask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that reads a file into a block of memory: open, read, and close.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x30 bytes. When
 * the work ends, OnStop() tells mOwner and does not record a result.
 */
class MCLoadFileTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e4b8
     * @ghidraAddress PAL: 0x0015fcf8
     */
    MCLoadFileTask();

    /**
     * Prepare the work.
     *
     * @param pOwner The task that learns when the work ends through OnFileLoaded(), or null.
     * @param nPort The memory card slot.
     * @param pszName The file.
     * @param pBuffer Receives the bytes. The caller retains it.
     * @param nSize The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x0015e600
     * @ghidraAddress PAL: 0x0015fe40
     */
    void Set(MemcardSerialTask *pOwner, int nPort, const char *pszName, void *pBuffer, int nSize);

    MemcardSerialTask *mOwner; /*!< The task OnStop() tells, or null. */
    MCOpenTask *mOpen;         /*!< The step that opens the file. */
    MCReadTask *mRead;         /*!< The step that reads the file. */
    MCCloseTask *mClose;       /*!< The step that closes the file. */

protected:
    /**
     * Tell mOwner that the work ended.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e5c8
     * @ghidraAddress PAL: 0x0015fe08
     */
    void OnStop() override;
};

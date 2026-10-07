#pragma once

#include "memcard/mcclosetask.h"
#include "memcard/mcopencurrremixtask.h"
#include "memcard/mcreadtask.h"
#include "memcard/mcseektask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that reads the remix after the RemixInfo of the file at g_RemixPath: open, skip the
 * description, read, and close.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x38 bytes.
 */
class MCGetRemixDataTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00161160
     * @ghidraAddress PAL: 0x00163ca0
     */
    MCGetRemixDataTask();

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pBuffer Receives the remix. The caller retains it.
     * @param nSize The number of bytes to read.
     * @ghidraAddress NTSC-U/C: 0x001612f0
     * @ghidraAddress PAL: 0x00163e30
     */
    void Set(int nPort, void *pBuffer, int nSize);

    void *mBuffer;              /*!< Receives the remix. */
    int mSize;                  /*!< The number of bytes to read. */
    MCOpenCurrRemixTask *mOpen; /*!< The step that opens the file. */
    MCSeekTask *mSeek;          /*!< The step that skips the description. */
    MCReadTask *mRead;          /*!< The step that reads the remix. */
    MCCloseTask *mClose;        /*!< The step that closes the file. */
};

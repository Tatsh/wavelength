#pragma once

#include "memcard/memcardtask.h"

/**
 * Task that reads bytes from the file MemcardTask::sFd identifies.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x18 bytes.
 */
class MCReadTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param pBuffer Receives the bytes. The caller retains it.
     * @param nSize The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x0015db58
     * @ghidraAddress PAL: 0x0015f390
     */
    void Set(void *pBuffer, int nSize);

    /**
     * Record the outcome and finish.
     *
     * @param nResult The number of bytes read, or a negative library error.
     * @ghidraAddress NTSC-U/C: 0x0015dba0
     * @ghidraAddress PAL: 0x0015f3d8
     */
    void OnRead(int nResult) override;

    void *mBuffer; /*!< Receives the bytes. */
    int mSize;     /*!< The number of bytes. */

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015dbf0
     * @ghidraAddress PAL: 0x0015f430
     */
    void OnStart() override;
};

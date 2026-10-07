#pragma once

#include "memcard/memcardtask.h"

/**
 * Task that writes bytes to the file MemcardTask::sFd identifies.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x18 bytes.
 */
class MCWriteTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param pData The bytes. The caller retains them.
     * @param nSize The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x0015da90
     * @ghidraAddress PAL: 0x0015f2c8
     */
    void Set(const void *pData, int nSize);

    /**
     * Record the outcome and finish.
     *
     * @param nResult The number of bytes written, or a negative library error.
     * @ghidraAddress NTSC-U/C: 0x0015dad8
     * @ghidraAddress PAL: 0x0015f310
     */
    void OnWrite(int nResult) override;

    const void *mData; /*!< The bytes. */
    int mSize;         /*!< The number of bytes. */

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015db28
     * @ghidraAddress PAL: 0x0015f360
     */
    void OnStart() override;
};

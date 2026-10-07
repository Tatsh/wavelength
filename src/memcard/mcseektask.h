#pragma once

#include "memcard/memcardtask.h"

/**
 * Task that moves the position of the file MemcardTask::sFd identifies.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x18 bytes.
 */
class MCSeekTask : public MemcardTask {
public:
    /**
     * Prepare the task. The slot is not changed.
     *
     * @param nOffset The offset.
     * @param nMode The origin of the offset.
     * @ghidraAddress NTSC-U/C: 0x0015dc20
     * @ghidraAddress PAL: 0x0015f460
     */
    void Set(int nOffset, int nMode);

    /**
     * Record the outcome and finish.
     *
     * @param nResult The new position, or a negative library error.
     * @ghidraAddress NTSC-U/C: 0x0015dc30
     * @ghidraAddress PAL: 0x0015f470
     */
    void OnSeek(int nResult) override;

    int mOffset; /*!< The offset. */
    int mMode;   /*!< The origin of the offset. */

protected:
    /**
     * Issue the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0015dc80
     * @ghidraAddress PAL: 0x0015f4c0
     */
    void OnStart() override;
};

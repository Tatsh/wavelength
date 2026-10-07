#pragma once

#include "memcard/mcopentask.h"
#include "memcard/memcardtask.h"

/**
 * Task that opens the remix file at g_RemixPath for reading.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x38 bytes.
 */
class MCOpenCurrRemixTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x001610c0
     * @ghidraAddress PAL: 0x00163c00
     */
    void Set(int nPort);

    MCOpenTask mOpen; /*!< The step that opens the file. */

protected:
    /**
     * Start opening the file.
     *
     * @ghidraAddress NTSC-U/C: 0x001610c8
     * @ghidraAddress PAL: 0x00163c08
     */
    void OnStart() override;

    /**
     * Finish with the outcome of the open.
     *
     * @ghidraAddress NTSC-U/C: 0x00161110
     * @ghidraAddress PAL: 0x00163c50
     */
    void OnPoll() override;
};

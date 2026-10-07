#pragma once

#include "memcard/mcdeletetask.h"
#include "memcard/memcardtask.h"

/**
 * Task that deletes the remix file at g_RemixPath.
 *
 * The RTTI records the class as deriving from MemcardTask. The object is 0x34 bytes.
 */
class MCDeleteSavedRemixTask : public MemcardTask {
public:
    /**
     * Prepare the task.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015fed8
     * @ghidraAddress PAL: 0x00162738
     */
    void Set(int nPort);

    MCDeleteTask mDelete; /*!< The step that deletes the file. */

protected:
    /**
     * Start deleting the file.
     *
     * @ghidraAddress NTSC-U/C: 0x0015fef8
     * @ghidraAddress PAL: 0x00162758
     */
    void OnStart() override;

    /**
     * Finish once the deletion ends.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ff38
     * @ghidraAddress PAL: 0x00162798
     */
    void OnPoll() override;
};
